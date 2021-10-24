#pragma once

#include "types.h"
#include "controller.h"

class CPU;
class APU;
class PPU;
class RAM;
class Cartridge;

class BUS
{
public:
    BUS(CPU& cpu, APU& apu, PPU& ppu, RAM& ram)
        : cpu_(cpu)
        , apu_(apu)
        , ppu_(ppu)
        , ram_(ram)
    {
    }

    void load_cartridge(Cartridge* cart)
    {
        cart_ = cart;
    }

    void write_cpu(address_t addr, byte_t value);
    byte_t read_cpu(address_t addr);

    void write_ppu(address_t addr, byte_t value);
    byte_t read_ppu(address_t addr);

    CPU& cpu_;
    APU& apu_;
    PPU& ppu_;
    RAM& ram_;
    Cartridge* cart_ = nullptr;
    Controller ctrl_;
};
