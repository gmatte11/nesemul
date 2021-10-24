#include "disassembler.h"

#include "ops.h"
#include "cartridge.h"

#include <algorithm>


void Disassembler::load(Cartridge* cart)
{
    cart_ = cart;

    for (Cartridge::PRG_BANK& bank : cart_->prg_rom_)
        load_bank(bank.data(), bank.size());
}

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

        if (data.operation == ops::kUKN)
        {
            ++op;
            continue;
        }

        address_t addr = static_cast<address_t>(op - rom);
        bank.ops_.push_back({addr, opcode});

        op += data.get_size();
    }

    bank.ops_.shrink_to_fit();
}

void Disassembler::render(fmt::memory_buffer& buf, address_t addr, int offset) const
{
    if (cart_ == nullptr)
        return;

    auto [rom, rom_addr] = cart_->get_bank(addr);

    auto it = std::find_if(banks_.begin(), banks_.end(), [rom = rom](PrgBank const& bank) { return bank.rom_ == rom; });
    if (it == banks_.end())
        return;

    PrgBank const& bank = *it;

    if (offset != 0)
    {
        auto it = std::find_if(bank.ops_.begin(), bank.ops_.end(), [addr = rom_addr](Op const& op) { return op.addr_ == addr; });
        if (it == bank.ops_.end()) 
            return;
        it += offset;
        addr = addr - (static_cast<int>(rom_addr) - it->addr_);
        rom_addr = it->addr_;
    }

    byte_t* op = bank.rom_ + rom_addr;
    ops::metadata const& data = ops::opcode_data(*op);

    fmt::format_to(buf, "{}{:04X}  {:02X} ", (offset == 0) ? '*': ' ', addr, *op);

    byte_t* operand1 = (data.get_size() >= 2) ? op + 1 : nullptr;
    byte_t* operand2 = (data.get_size() == 3) ? op + 2 : nullptr;

    fmt::format_to(buf, (operand1) ? "{:02X} " : "   ", (operand1) ? *operand1 : 0);
    fmt::format_to(buf, (operand2) ? "{:02X} " : "   ", (operand2) ? *operand2 : 0);

    address_t read_addr = 0;
    if (operand1 && operand2)
        read_addr = static_cast<address_t>(*operand2) << 8 | *operand1;

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
}