#include "mapper.h"

#include "mappers/000.h"
#include "mappers/001.h"

Mapper* Mapper::create(byte_t ines_code)
{
    switch (ines_code)
    {
    case 0: return new M000;
    case 1: return new M001;
    }

    return nullptr;
}

void Mapper::init(std::vector<PRG_BANK> && prg_rom, std::vector<CHR_BANK> && chr_rom)
{
    prg_rom_ = std::move(prg_rom);
    chr_rom_ = std::move(chr_rom);
    init_ptrs_();
}

void Mapper::init_ptrs_()
{
    prg_l_ = prg_rom_[0].data();
    prg_h_ = (prg_rom_.size() > 1) ? prg_rom_[1].data() : prg_l_;

    chr_l_ = chr_rom_[0].data();
    chr_h_ = chr_rom_[0].data() + 0x1000;
}