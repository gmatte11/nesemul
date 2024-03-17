#include "disassembler.h"

#include "ops.h"
#include "cartridge.h"
#include "utils.h"

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

void Disassembler::render(StringBuilder& sb, address_t addr, int offset) const
{
    if (cart_ == nullptr)
        return;

    auto [rom, rom_addr] = cart_->get_bank(addr);

    auto it_banks = std::find_if(banks_.begin(), banks_.end(), [rom = rom](PrgBank const& bank) { return bank.rom_ == rom; });
    if (it_banks == banks_.end())
        return;

    PrgBank const& bank = *it_banks;

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

    sb.append_fmt("{}{:04X}  {:02X} ", (offset == 0) ? '*': ' ', addr, *op);

    auto print_operand = [&sb](byte_t* op)
    {
        if (op != nullptr)
            sb.append_fmt("{:02X} ", *op);
        else
            sb.append(fmt::string_view("   "));
    };

    byte_t* operand1 = (data.get_size() >= 2) ? op + 1 : nullptr;
    byte_t* operand2 = (data.get_size() == 3) ? op + 2 : nullptr;

    print_operand(operand1);
    print_operand(operand2);

    address_t read_addr = 0;
    if (operand1 && operand2)
        read_addr = static_cast<address_t>(*operand2) << 8 | *operand1;

    sb.append_fmt(" {: >4}", data.str);
    switch(data.addressing)
    {
        case ops::Addressing::kImmediate:
            sb.append_fmt("#${:02x}", *operand1);
            break;

        case ops::Addressing::kZeroPage:
            sb.append_fmt("${:02x}", *operand1);
            break;

        case ops::Addressing::kZeroPageX:
            sb.append_fmt("${:02x},X", *operand1);
            break;

        case ops::Addressing::kAbsolute:
            sb.append_fmt("${:04x}", read_addr);
            break;

        case ops::Addressing::kIndirect:
            sb.append_fmt("(${:04x})", read_addr);
            break;

        case ops::Addressing::kIndirectX:
            sb.append_fmt("(${:02x}),X", *operand1);
            break;

        default:
            break;
    }
}