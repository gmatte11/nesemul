#pragma once
#include "types.h"

#include <vector>
#include <string>

#include <fmt/format.h>

class Disassembler
{
public:
    Disassembler() = default;

    void load_bank(byte_t* rom, size_t size = 0x4000);
    void set_current_bank(byte_t* rom);

    std::string render(fmt::memory_buffer& buf, address_t addr, int offset = 0) const;

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
    size_t current_bank_idx_ = 0;
};

