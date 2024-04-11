#pragma once

#include "types.h"

#include <array>
#include <vector>

class Cartridge;

class Mapper
{
public:
    static Mapper* create(byte_t ines_code, Cartridge* cart);

public:
    Mapper(Cartridge* cart) : cart_(cart) {}

    void post_load();

    virtual address_t map_to_cpu_addr(address_t addr) const = 0;    

    virtual bool on_cpu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_cpu_write(address_t addr, byte_t value) = 0;

    virtual bool on_ppu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_ppu_write(address_t addr, byte_t value) = 0;

    virtual std::pair<byte_t*, address_t> get_bank(address_t addr) const = 0;

    std::pair<const byte_t*, const byte_t*> get_cpu_mapped_prg_banks() const
    {
        return { prg_l_, prg_h_ };
    }

    std::pair<const byte_t*, const byte_t*> get_ppu_mapped_chr_banks() const
    {
        return { chr_l_, chr_h_ };
    }

protected:
    Cartridge* cart_;

    byte_t* prg_l_;
    byte_t* prg_h_;
    byte_t* chr_l_;
    byte_t* chr_h_;
};