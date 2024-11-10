#include "000.h"

M000::M000(Cartridge& cart)
{
    prg_l_ = { cart.get_prg_bank(0), 0x8000 };
    prg_map_[0] = prg_l_;

    prg_h_ = {cart.get_prg_bank(-1), 0xC000};
    prg_map_[1] = prg_h_;

    chr_ = {cart.get_chr_bank(0), 0x0000};
    chr_map_[0] = chr_;
}

bool M000::on_cpu_read(address_t addr, byte_t& value) 
{
    if (addr >= 0x8000 && addr < 0xC000)
    {
        prg_l_.read(addr & 0x3FFF, value);
        return true;
    }

    if (addr >= 0xC000)
    {
        prg_h_.read(addr & 0x3FFF, value);
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
        chr_.read(addr & 0x1FFF, value);
        return true;
    }

    return false;
}

bool M000::on_ppu_write(address_t addr, byte_t value)
{
    return false;
}

address_t M000::map_to_cpu_addr(address_t addr) const
{
    if (prg_h_.contains(addr) && prg_h_.is_mirror_of(prg_l_))
        return addr - 0x4000;

    return addr;
}
