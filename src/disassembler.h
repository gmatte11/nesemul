#pragma once
#include "types.h"

#include <vector>

#include <fmt/format.h>

class Cartridge;

class Disassembler
{
public:
    Disassembler() = default;

    void load(Cartridge* cart);
    void load_bank(byte_t* rom, size_t size = 0x4000);

    void render(fmt::memory_buffer& buf, address_t addr, int offset = 0) const;

private:
    struct Op
    {
        address_t addr_;
        byte_t opcode_;
    };

    struct PrgBank
    {
        byte_t* rom_;
        std::vector<Op> ops_;
    };

    std::vector<PrgBank> banks_;
    Cartridge* cart_;
};

