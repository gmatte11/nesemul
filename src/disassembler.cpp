#include "disassembler.h"

#include "ops.h"

#include <algorithm>

DEOPTIMIZE
void Disassembler::load_bank(byte_t* rom, size_t size)
{
    PrgBank& bank = banks_.emplace_back();
    bank.rom_ = rom;
    bank.ops_.reserve(size / 2);

    byte_t* op = rom;
    byte_t* end = rom + size;
    while (op < end)
    {
        byte_t opcode = *op;
        ops::metadata const& data = ops::opcode_data(opcode);

        if (data.size == 0)
        {
            ++op;
            continue;
        }

        address_t addr = static_cast<address_t>(op - rom);
        bank.ops_.push_back({addr, opcode});

        op += data.size;
    }

    bank.ops_.shrink_to_fit();
}

void Disassembler::set_current_bank(byte_t* rom)
{
    for (int i = 0; i < banks_.size(); ++i)
    {
        if (rom == banks_[i].rom_)
        {
            current_bank_idx_ = i;
            break;
        }
    }
}

std::string Disassembler::render(fmt::memory_buffer& buf, address_t addr, int offset) const
{
    PrgBank const& bank = banks_[current_bank_idx_];

    if (offset != 0)
    {
        auto it = std::find_if(bank.ops_.begin(), bank.ops_.end(), [addr](Op const& op) { return op.addr_ == addr; });
        it += offset;
        addr = it->addr_;
    }

    byte_t* op = bank.rom_ + addr;
    ops::metadata const& data = ops::opcode_data(*op);

    fmt::format_to(buf, "{:04X}  {:02X} ", addr, *op);

    byte_t* operand1 = (data.size >= 2) ? op + 1 : nullptr;
    byte_t* operand2 = (data.size == 3) ? op + 2 : nullptr;

    fmt::format_to(buf, (operand1) ? "{:02X} " : "   ", (operand1) ? *operand1 : 0);
    fmt::format_to(buf, (operand2) ? "{:02X} " : "   ", (operand2) ? *operand2 : 0);

    address_t read_addr = 0;
    if (operand1 && operand2)
        read_addr = static_cast<address_t>(*operand2) << 8 || *operand1;

    fmt::format_to(buf, " {: >4}", data.str);
    switch(data.addressing)
    {
        case ops::Addressing::kImmediate:
            fmt::format_to(buf, "#${:02x}", *operand1);
            break;

        case ops::Addressing::kZeroPage:
            fmt::format_to(buf, "${:02x}", *operand1);
            break;

        case ops::Addressing::kZeroPageX:
            fmt::format_to(buf, "${:02x},X", *operand1);
            break;

        case ops::Addressing::kAbsolute:
            fmt::format_to(buf, "${:04x}", read_addr);
            break;

        case ops::Addressing::kIndirect:
            fmt::format_to(buf, "(${:04x})", read_addr);
            break;

        case ops::Addressing::kIndirectX:
            fmt::format_to(buf, "(${:02x}),X", *operand1);
            break;

        default:
            break;
    }

    return std::string{buf.data()};
}