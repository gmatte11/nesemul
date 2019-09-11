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
};

Emulator::Emulator()
    : cpu_(new CPU)
    , ppu_(new PPU)
    , ram_(new RAM)
{
    bus_.reset(new BUS(*cpu_, *ppu_, *ram_));
    cpu_->init(bus_.get());
    ppu_->init(bus_.get());
}

void Emulator::read(const std::string& filename)
{
    std::ifstream ifs(filename, std::ios_base::binary);

    if (ifs)
    {
        char header[4];
        ifs.read(header, 4);
        if (std::memcmp(header, "NES", 3) != 0 && header[3] != 0x1a)
            throw std::runtime_error(std::string("Bad file format"));

        char num_16kb_rom_banks;
        ifs.read(&num_16kb_rom_banks, 1);
        std::cout << "num_16kb_rom_banks: " << (int)num_16kb_rom_banks << std::endl;

        char num_8kb_vrom_banks;
        ifs.read(&num_8kb_vrom_banks, 1);
        std::cout << "num_8kb_vrom_banks: " << (int)num_8kb_vrom_banks << std::endl;

        char config[2];
        ifs.read(config, 2);
        std::cout << "config:\n"
            << "    mirroring: " << ((0x1 & config[0]) ? "vertical" : "horizontal") << "\n"
            << "    battery: " << ((0x2 & config[0]) >> 1) << "\n"
            << "    trainer: " << ((0x4 & config[0]) >> 2) << "\n"
            << "    four-screen: " << ((0x8 & config[0]) >> 3) << "\n"
            << "    VS-System cartige: " << ((0x1 & config[1])) << "\n";

        byte_t mapper = (byte_t)(config[1] & 0xf0 + (config[0] & 0xf0 >> 4));
        std::cout << "mapper: " << (int)mapper << std::endl;

        Mirroring mir = Mirroring::None;
        if (!((0x8 & config[0]) >> 3))
        {
            mir = (0x1 & config[0]) ? Mirroring::Vertical : Mirroring::Horizontal;
        }
        ppu_->set_mirroring(mir);

        char num_8kb_ram_banks;
        ifs.read(&num_8kb_ram_banks, 1);
        if (num_8kb_ram_banks == 0)
            num_8kb_ram_banks = 1;
        std::cout << "num_8kb_ram_banks: " << (int)num_8kb_ram_banks << std::endl;

        char cartige_type;
        ifs.read(&cartige_type, 1);
        std::cout << "cartige_type: " << (cartige_type >> 7) << std::endl;

        ifs.seekg(16);

        // trainer is 512B, skip it for now
        if (((0x4 & config[0]) >> 2) != 0)
            ifs.seekg(16 + 512);

        std::stringstream ss;
        ss << ifs.rdbuf();
        std::string buf{ss.str()};

        // Program rom (PRG-ROM) is loaded in $8000
        std::memcpy(ram_->data() + 0x8000, buf.data(), 0x8000);

        if (num_16kb_rom_banks == 1)
            std::memcpy(ram_->data() + 0xC000, buf.data(), 0x4000);

        // Character rom (CHR-ROM) is loaded in ppu $0000
        std::memcpy(ppu_->data(), buf.data() + (0x4000 * num_16kb_rom_banks), 0x2000);
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
