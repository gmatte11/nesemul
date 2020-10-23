#include "cartridge.h"

#include <fstream>

Cartridge::Cartridge(byte_t ines_mapper_code)
{
    mapper_ = Mapper::create(ines_mapper_code, this);
}

void Cartridge::load_roms(bifstream& ifs, byte_t prg_rom_banks, byte_t chr_rom_banks, byte_t prg_ram_banks)
{
    // Program rom (PRG-ROM)
    if (prg_rom_banks > 0)
    {
        prg_rom_.reserve(prg_rom_banks);
        byte_t* prg_buf = new byte_t[prg_rom_banks * 0x4000];
        ifs.read(prg_buf, prg_rom_banks * 0x4000);
        byte_t* cur = prg_buf;

        for (int i = 0; i < prg_rom_banks; ++i)
        {
            auto& bank = prg_rom_.emplace_back();
            std::memcpy(bank.data(), cur, 0x4000);
            cur += 0x4000;
        }
        delete[] prg_buf;
    }

    // Character rom (CHR-ROM)
    if (chr_rom_banks > 0)
    {
        chr_rom_.reserve(chr_rom_banks);
        byte_t* chr_buf = new byte_t[chr_rom_banks * 0x2000];
        ifs.read(chr_buf, chr_rom_banks * 0x2000);
        byte_t* cur = chr_buf;

        for (int i = 0; i < chr_rom_banks; ++i)
        {
            auto& bank = chr_rom_.emplace_back();
            std::memcpy(bank.data(), cur, 0x2000);
            cur += 0x2000;
        }

        delete[] chr_buf;
    }

    mapper_->post_load();
}

std::pair<byte_t*, address_t> Cartridge::get_bank(address_t addr) const
{
    return mapper_->get_bank(addr);
}