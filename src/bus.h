#pragma once

#include "types.h"
#include "controller.h"

class CPU;
class PPU;
class RAM;

class BUS
{
public:
    BUS(CPU& cpu, PPU& ppu, RAM& ram)
        : cpu_(cpu)
        , ppu_(ppu)
        , ram_(ram)
    {
    }

    void write(address_t addr, byte_t value);
    byte_t read(address_t addr);

    CPU& cpu_;
    PPU& ppu_;
    RAM& ram_;
    Controller ctrl_;
};
