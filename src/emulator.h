#pragma once

#include <memory>
#include <string>

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"
#include "disassembler.h"

class Emulator
{
public:
    Emulator();

    void read(const std::string& filename);

    int run();

    Disassembler disassembler_;
    BUS* get_bus() const { return bus_.get(); }

private:
    std::unique_ptr<BUS> bus_;
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<RAM> ram_;
    std::unique_ptr<Cartridge> cart_;
};