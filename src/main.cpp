#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstring>

#include <types.h>
#include <bus.h>
#include <cpu.h>
#include <ppu.h>
#include <sdl_renderer.h>

class RAM : public Device
{
public:
    RAM(BUS& bus) { bus.register_device(this); }

    bool on_write(address_t addr, byte_t value) override { memory_[addr] = value; return true; }
    bool on_read(address_t addr, byte_t& value) override { value = memory_[addr]; return true; }

private:
    std::array<byte_t, 0x10000> memory_{};
};

class Emulator
{
public:
    Emulator();

    void read(const std::string& filename);

    int run();

private:
    BUS bus_;
    CPU cpu_;
    PPU ppu_;
    RAM ram_;
};

Emulator::Emulator()
    : cpu_(bus_)
    , ppu_(bus_, cpu_)
    , ram_(bus_)
{
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
        ppu_.set_mirroring(mir);

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
        std::memcpy(cpu_.data() + 0x8000, buf.data(), 0x8000);

        if (num_16kb_rom_banks == 1)
            std::memcpy(cpu_.data() + 0xC000, buf.data(), 0x4000);

        // Character rom (CHR-ROM) is loaded in ppu $0000
        std::memcpy(ppu_.data(), buf.data() + (0x4000 * num_16kb_rom_banks), 0x2000);
    }
    else
    {
        throw std::runtime_error(std::string("Can't open file ") + filename);
    }
}

int Emulator::run()
{
    SDLRenderer renderer;

    cpu_.reset();
    ppu_.reset();

    for (;;)
    {
        cpu_.next();
        for (int i = 0; i < 3; ++i) ppu_.next();

        if (renderer.timeout())
        {
            if (!renderer.update(ppu_))
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
