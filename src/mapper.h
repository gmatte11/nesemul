#pragma once

#include "types.h"

#include <array>
#include <vector>

class Mapper
{
public:
    static Mapper* create(byte_t ines_code);

    virtual bool on_cpu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_cpu_write(address_t addr, byte_t value) = 0;

    virtual bool on_ppu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_ppu_write(address_t addr, byte_t value) = 0;

    using PRG_BANK = std::array<byte_t, 0x4000>;
    std::vector<PRG_BANK> prg_rom_;

    using CHR_BANK = std::array<byte_t, 0x2000>;
    std::vector<CHR_BANK> chr_rom_;
};