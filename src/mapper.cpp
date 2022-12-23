#include "mapper.h"
#include "cartridge.h"

#include "mappers/000.h"
#include "mappers/001.h"

#include <fmt/core.h>
#include <stdexcept>

Mapper* Mapper::create(byte_t ines_code, Cartridge* cart_)
{
    switch (ines_code)
    {
    case 0: return new M000(cart_);
    case 1: return new M001(cart_);
    default:
        throw std::runtime_error(fmt::format("Unknown mapper {:03}", ines_code));
    }

    return nullptr;
}

void Mapper::post_load()
{
    prg_l_ = cart_->prg_rom_.front().data();
    prg_h_ = cart_->prg_rom_.back().data();
    
    // A single bank of CHR RAM is created when no CHR ROM is available in the cartridge.
    if (cart_->chr_.empty())
        cart_->chr_.emplace_back();

    chr_l_ = cart_->chr_.front().data();
    chr_h_ = chr_l_ + 0x1000;
}