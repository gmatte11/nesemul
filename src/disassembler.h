#pragma once
#include "types.h"

#include <vector>
#include <span>

class BUS;
class Cartridge;
struct StringBuilder;

class Disassembler
{
public:
    struct Op
    {
        address_t addr_;
        byte_t opcode_;
        byte_t operand1_;
        byte_t operand2_;
    };

    struct PrgBank
    {
        std::vector<Op> ops_;
        const byte_t* rom_;
        int rom_size_;
        int rom_bank_idx_; // Source prg rom index on the cartridge
    };
    
    static constexpr size_t rom_size = 0x4000;

    Disassembler() = default;

    void load(const BUS& bus);
    void render(StringBuilder& buf, address_t addr, int offset = 0) const;
    void asm_str(StringBuilder& sb, Op op) const;

    int get_ops_count(const byte_t* bank_mem, int size);
    std::span<const PrgBank> get_banks() const { return banks_; }

private:
    void load_bank_(PrgBank& bank, const byte_t* rom, size_t size);

    std::vector<PrgBank> banks_;
    Cartridge* cart_ = nullptr;
};

