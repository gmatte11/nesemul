#include "000.h"

bool M000::on_cpu_read(address_t addr, byte_t& value) 
{
    if (addr >= 0xC000)
    {
        if (prg_rom_.size() >= 2)
        {
            value = prg_rom_[1][addr - 0xC000];
            return true;
        }
        addr -= 0x4000; // mirror to range $8000-$C000
    } 

    if (addr >= 0x8000 && addr < 0xC000)
    {
        value = prg_rom_[0][addr - 0x8000];
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
        value = chr_rom_[0][addr];
        return true;
    }

    return false;
}

bool M000::on_ppu_write(address_t addr, byte_t value)
{
    return false;
}