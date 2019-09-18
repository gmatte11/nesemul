#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <memory>

#include "types.h"
#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"
#include "sfml_renderer.h"

class Emulator
{
public:
    Emulator();

    void read(const std::string& filename);

    int run();

private:
    std::unique_ptr<BUS> bus_;
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<RAM> ram_;
    std::unique_ptr<Cartridge> cart_;
};

Emulator::Emulator()
    : cpu_(new CPU)
    , ppu_(new PPU)
    , ram_(new RAM)
    , cart_(new Cartridge)
{
    bus_.reset(new BUS(*cpu_, *ppu_, *ram_, *cart_));
    cpu_->init(bus_.get());
    ppu_->init(bus_.get(), cart_.get());
}

void Emulator::read(const std::string& filename)
{
    using bifstream = std::basic_ifstream<byte_t>;
    bifstream ifs(filename, std::ios_base::binary);

    if (ifs)
    {
        byte_t header[16];
        ifs.read(header, 16);

        if (std::memcmp(header, "NES", 3) != 0 && header[3] != 0x1a)
            throw std::runtime_error("Bad file format");

        bool nes20 = (0x3 & (header[7] >> 2)) == 2;
        if (nes20)
            throw std::runtime_error("NES 2.0 header not yet supported.");

        byte_t num_16kb_prg_rom_banks = header[4];
        std::cout << "num_16kb_prg_rom_banks: " << (int)num_16kb_prg_rom_banks << std::endl;

        byte_t num_8kb_chr_rom_banks = header[5];
        std::cout << "num_8kb_chr_rom_banks: " << (int)num_8kb_chr_rom_banks << std::endl;

        Mirroring mir = Mirroring::None;
        if ((0x1 & (header[6] >> 3)) == 0)
        {
            mir = (0x1 & header[6]) ? Mirroring::Vertical : Mirroring::Horizontal;
        }

        bool battery = 0x1 & (header[6] >> 1);
        bool trainer = 0x1 & (header[6] >> 2);
        bool fourscreen = 0x1 & (header[6] >> 3);

        byte_t mapper = (byte_t)((header[7] & 0xF0) | (header[6] >> 4));
        std::cout << "mapper: " << (int)mapper << std::endl;

        std::cout << "config:\n"
                  << "    mirroring: " << (mir == Mirroring::Vertical ? "vertical" : "horizontal") << "\n"
                  << "    battery: " << battery << "\n"
                  << "    trainer: " << trainer << "\n"
                  << "    four-screen: " << fourscreen << "\n"
                  << "    VS-System cartige: " << (0x1 & (header[7] >> 0)) << "\n"
                  << "    PlayChoice-10: " << (0x1 & (header[7] >> 1)) << "\n";

        cart_->mapper_ = Mapper::create(mapper);
        if (cart_->mapper_ == nullptr)
        {
            throw std::runtime_error(std::string("Unknown mapper ") + filename);
        }

        ppu_->set_mirroring(mir);

        byte_t num_8kb_prg_ram_banks = header[8];
        if (num_8kb_prg_ram_banks == 0)
            num_8kb_prg_ram_banks = 1; // compatibility

        //if (0x1 & (header[10] >> 4))
        //    num_8kb_prg_ram_banks = 1; // ??
        std::cout << "num_8kb_prg_ram_banks: " << (int)num_8kb_prg_ram_banks << std::endl;

        ifs.seekg(16);

        // trainer is 512B, skip it for now
        if (trainer)
            ifs.ignore(512);

        // Program rom (PRG-ROM)
        std::vector<Mapper::PRG_BANK> prg_rom;
        prg_rom.reserve(num_16kb_prg_rom_banks);
        {
            byte_t* prg_buf = new byte_t[num_16kb_prg_rom_banks * 0x4000];
            ifs.read(prg_buf, num_16kb_prg_rom_banks * 0x4000);
            byte_t* cur = prg_buf;

            for (int i = 0; i < num_16kb_prg_rom_banks; ++i)
            {
                auto& bank = prg_rom.emplace_back();
                std::memcpy(bank.data(), cur, 0x4000);
                cur += 0x4000;
            }
            delete[] prg_buf;
        }

        // Character rom (CHR-ROM)
        std::vector<Mapper::CHR_BANK> chr_rom;
        chr_rom.reserve(num_8kb_chr_rom_banks);
        {
            byte_t* chr_buf = new byte_t[num_8kb_chr_rom_banks * 0x2000];
            ifs.read(chr_buf, num_8kb_chr_rom_banks * 0x2000);
            byte_t* cur = chr_buf;

            for (int i = 0; i < num_8kb_chr_rom_banks; ++i)
            {
                auto& bank = chr_rom.emplace_back();
                std::memcpy(bank.data(), cur, 0x2000);
                cur += 0x2000;
            }

            delete[] chr_buf;
        }

        cart_->mapper_->init(std::move(prg_rom), std::move(chr_rom));
    }
    else
    {
        throw std::runtime_error(std::string("Can't open file ") + filename);
    }
}

int Emulator::run()
{
    SFMLRenderer renderer(bus_.get());

    cpu_->reset();
    ppu_->reset();

    for (;;)
    {
        if (renderer.timeout())
        {
            if (!renderer.is_paused())
            {
                // NTSC emulation: 29780.5 cpu cycles per frame: ~60 Hz
                for (int i = 0; i < 29780; ++i)
                {
                    cpu_->next();
                    for (int i = 0; i < 3; ++i)
                        ppu_->next();
                }
            }

            if (!renderer.update(*ppu_))
                break;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;

    try
    {
        Emulator emul;
        emul.read(argv[1]);

        return emul.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
