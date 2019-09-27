#include "mapper.h"
#include "cartridge.h"

#include "mappers/000.h"
#include "mappers/001.h"

Mapper* Mapper::create(byte_t ines_code, Cartridge* cart_)
{
    switch (ines_code)
    {
    case 0: return new M000(cart_);
    case 1: return new M001(cart_);
    }

    return nullptr;
}

void Mapper::post_load()
{
    prg_l_ = cart_->prg_rom_.front().data();
    prg_h_ = cart_->prg_rom_.back().data();
    
    if (!cart_->chr_rom_.empty())
    {
        chr_l_ = cart_->chr_rom_.front().data();
    }
    else
    {
        chr_l_ = cart_->chr_ram_.data(); 
    }
    chr_h_ = chr_l_ + 0x1000;
}