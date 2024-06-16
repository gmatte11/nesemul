#include "000.h"

M000::M000()
    : prg_l_(prg_map_.map_[0].mem_)
    , prg_h_(prg_map_.map_[1].mem_)
    , chr_l_(chr_map_.map_[0].mem_)
    , chr_h_(chr_map_.map_[1].mem_)
{
    prg_map_[0].addr_ = 0x8000;
    prg_map_[0].size_ = 0x4000;
    prg_map_[1].addr_ = 0xC000;
    prg_map_[1].size_ = 0x4000;

    chr_map_[0].addr_ = 0x0000;
    chr_map_[0].size_ = 0x1000;
    chr_map_[1].addr_ = 0x1000;
    chr_map_[1].size_ = 0x1000;
}

void M000::post_load(Cartridge& cart)
{
    prg_l_ = cart.get_prg_bank(0);
    prg_h_ = cart.get_prg_bank(-1);
    
    chr_l_ = cart.get_chr_bank(0);
    chr_h_ = chr_l_ + 0x1000;
}

address_t M000::map_to_cpu_addr(address_t addr) const
{
    if (prg_h_ == prg_l_ && addr >= 0xC000)
        return addr -= 0x4000;

    return addr;
}

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
        return  std::pair((addr < 0xC000) ? prg_l_ : prg_h_, static_cast<address_t>(addr & 0x3FFF));
    }

    return std::pair(nullptr, 0_addr);
}