#pragma once

#include "mapper.h"

// SxROM
class M001 : public Mapper
{
public:
    M001(Cartridge* cart) : Mapper(cart) {}

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

private:
    byte_t register_ = 0b10000;
    byte_t control_ = 0;
    
    void chr_switch(bool low);
    void prg_switch();
};