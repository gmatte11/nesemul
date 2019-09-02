#pragma once

#include <types.h>

class CPU;
class PPU;
class RAM;

class BUS
{
public:
    BUS(CPU& cpu, PPU& ppu, RAM& ram);

    void write(address_t addr, byte_t value);
    byte_t read(address_t addr);

    CPU& cpu_;
    PPU& ppu_;
    RAM& ram_;
};
