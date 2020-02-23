#include "000.h"

bool M000::on_cpu_read(address_t addr, byte_t& value) 
{
    if (addr >= 0x8000 && addr < 0xC000)
    {
        value = *(prg_l_ + (addr & 0x3FFF));
        return true;
    }

    if (addr >= 0xC000)
    {
        value = *(prg_h_ + (addr & 0x3FFF));
        return true;
    }

    return false;
}

bool M000::on_cpu_write(address_t addr, byte_t value)
{
    return false;
}

bool M000::on_ppu_read(address_t addr, byte_t& value)
{
    if (addr < 0x2000)
    {
        value = *(chr_l_ + addr);
        return true;
    }

    return false;
}

bool M000::on_ppu_write(address_t addr, byte_t value)
{
    return false;
}

std::pair<byte_t*, address_t> M000::get_bank(address_t addr) const
{
    if (addr >= 0x8000)
    {
        return  std::make_pair((addr < 0xC000) ? prg_l_ : prg_h_, addr & 0x3FFF);
    }

    return std::make_pair(nullptr, 0u);
}