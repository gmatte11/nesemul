#pragma once

#include "types.h"

#include <cstring>

class RAM
{
public:
    bool on_write(address_t addr, byte_t value) 
    { 
        if (addr < 0x2000)
        {
            memory_[addr & 0x7FF] = value; 
            return true; 
        }

        return false;
    }

    bool on_read(address_t addr, byte_t& value) 
    {
        if (addr < 0x2000)
        {
            value = memory_[addr & 0x7FF]; 
            return true;
        }

        return false;
    }

    byte_t* data() { return memory_.data(); }

private:
    // 2KB for internal RAM
    std::array<byte_t, 0x800> memory_{};
};