#pragma once

#include "types.h"

#include <array>
#include <vector>

class Mapper
{
public:
    static Mapper* create(byte_t ines_code);

public:
    using PRG_BANK = std::array<byte_t, 0x4000>;
    using CHR_BANK = std::array<byte_t, 0x2000>;

    void init(std::vector<PRG_BANK> && prg_rom, std::vector<CHR_BANK> && chr_rom);

    virtual bool on_cpu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_cpu_write(address_t addr, byte_t value) = 0;

    virtual bool on_ppu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_ppu_write(address_t addr, byte_t value) = 0;

protected:
    std::vector<PRG_BANK> prg_rom_;
    std::vector<CHR_BANK> chr_rom_;

    virtual void init_ptrs_();
    byte_t* prg_l_;
    byte_t* prg_h_;
    byte_t* chr_l_;
    byte_t* chr_h_;
};