#include "cartridge.h"

#include "ines.h"

Cartridge::Cartridge(byte_t ines_mapper_code)
{
    mapper_ = Mapper::create(ines_mapper_code, this);
}

void Cartridge::load_roms(INESReader& reader)
{
    INESHeader& h = reader.header_;

    // 16 kb Program rom banks (PRG-ROM)
    if (h.prg_rom_size_ > 0)
    {
        prg_rom_.reserve(h.prg_rom_size_);

        for (int i = 0; i < h.prg_rom_size_; ++i)
        {
            auto& bank = prg_rom_.emplace_back();
            reader.read_prg_rom(i, bank);
        }
    }

    // 8 kb Character rom banks (CHR-ROM)
    if (h.chr_rom_size_ > 0)
    {
        chr_.reserve(h.chr_rom_size_);

        for (int i = 0; i < h.chr_rom_size_; ++i)
        {
            auto& bank = chr_.emplace_back();
            reader.read_chr_rom(i, bank);
        }
    }

    mapper_->post_load();
}

std::pair<byte_t*, address_t> Cartridge::get_bank(address_t addr) const
{
    return mapper_->get_bank(addr);
}