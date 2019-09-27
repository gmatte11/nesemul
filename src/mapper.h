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

    virtual bool on_cpu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_cpu_write(address_t addr, byte_t value) = 0;

    virtual bool on_ppu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_ppu_write(address_t addr, byte_t value) = 0;

protected:
    Cartridge* cart_;

    byte_t* prg_l_;
    byte_t* prg_h_;
    byte_t* chr_l_;
    byte_t* chr_h_;
};