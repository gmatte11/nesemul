#pragma once

#include "types.h"

#include <cstring>

class RAM
{
public:
    bool on_write(address_t addr, byte_t value) { memory_[addr] = value; return true; }
    bool on_read(address_t addr, byte_t& value) { value = memory_[addr]; return true; }

    void memcpy(void* dest, address_t src, int size) { std::memcpy(dest, memory_.data() + src, size); }

    byte_t* data() { return memory_.data(); }

private:
    std::array<byte_t, 0x10000> memory_{};
};