#pragma once

#include "types.h"
#include "mapper.h"

class Cartridge
{
public:
    bool on_cpu_read(address_t addr, byte_t& value) 
    { 
        if (addr >= 0x4020)
            return mapper_->on_cpu_read(addr, value);
        return false;
    }

    bool on_cpu_write(address_t addr, byte_t value) 
    { 
        if (addr >= 0x4020)
            return mapper_->on_cpu_write(addr, value); 
        return false;
    }

    bool on_ppu_read(address_t addr, byte_t& value) { return mapper_->on_ppu_read(addr, value); }
    bool on_ppu_write(address_t addr, byte_t value) { return mapper_->on_ppu_write(addr, value); }

    Mapper* mapper_;
};