#include "disassembler.h"

#include "ops.h"
#include "bus.h"
#include "cartridge.h"
#include "utils.h"

#include <algorithm>
#include <ranges>


void Disassembler::load(const BUS& bus)
{
    Cartridge* cart = bus.cart_;

    banks_.clear();

    if (cart == nullptr)
        return;

    banks_.reserve(cart->prg_rom_.size());

    for (size_t i = 0; i < cart->prg_rom_.size(); ++i)
    {
        const auto& prg_bank = cart->prg_rom_[i];

        PrgBank& bank = banks_.emplace_back();
        bank.rom_ = prg_bank.data();
        bank.source_idx = static_cast<int>(i);

        load_bank_(bank, bank.rom_, rom_size);
    }
}

void Disassembler::load_bank_(PrgBank& bank, const byte_t* rom, size_t size)
{
    const byte_t* op = rom;
    const byte_t* end = rom + size;
    while (op < end)
    {
        byte_t opcode = *op;
        ops::metadata const& meta = ops::opcode_data(opcode);

        if (meta.operation == ops::kUKN)
        {
            ++op;
            continue;
        }

        address_t addr = static_cast<address_t>(op - rom);
        byte_t operand1 = (meta.get_size() >= 2) ? *(op + 1) : 0;
        byte_t operand2 = (meta.get_size() == 3) ? *(op + 2) : 0; 

        bank.ops_.emplace_back(addr, opcode, operand1, operand2);

        op += meta.get_size();
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

    auto it = std::ranges::find_if(bank.ops_, [=](Op op) { return op.addr_ == addr; });
    it += offset;

    Op op = *it;

    ops::metadata const& data = ops::opcode_data(op.opcode_);

    sb.append_fmt("{}{:04X}  {:02X} ", (offset == 0) ? '*': ' ', addr, op.opcode_);

    auto print_operand = [&sb](const byte_t* op)
    {
        if (op != nullptr)
            sb.append_fmt("{:02X} ", *op);
        else
            sb.append(fmt::string_view("   "));
    };

    const byte_t* operand1 = (data.get_size() >= 2) ? &op.operand1_ : nullptr;
    const byte_t* operand2 = (data.get_size() == 3) ? &op.operand2_ : nullptr;

    print_operand(operand1);
    print_operand(operand2);

    asm_str(sb, op);
}

void Disassembler::asm_str(StringBuilder& sb, Op op) const
{
    const ops::metadata& meta = ops::opcode_data(op.opcode_);

    address_t read_addr = 0;
    if (meta.get_size() == 3)
        read_addr = static_cast<address_t>(op.operand2_) << 8 | op.operand1_;

    sb.append_fmt("{: >4}", meta.str);
    switch(meta.addressing)
    {
        case ops::Addressing::kImmediate:
            sb.append_fmt("#${:02x}", op.operand1_);
            break;

        case ops::Addressing::kZeroPage:
            sb.append_fmt("${:02x}", op.operand1_);
            break;

        case ops::Addressing::kZeroPageX:
            sb.append_fmt("${:02x},X", op.operand1_);
            break;

        case ops::Addressing::kAbsolute:
            sb.append_fmt("${:04x}", read_addr);
            break;

        case ops::Addressing::kIndirect:
            sb.append_fmt("(${:04x})", read_addr);
            break;

        case ops::Addressing::kIndirectX:
            sb.append_fmt("(${:02x}),X", op.operand1_);
            break;

        default:
            break;
    }
}

auto Disassembler::get_mapped_banks(const BUS& bus) -> std::pair<const PrgBank*, const PrgBank*>
{
    if (bus.cart_ != nullptr)
    {
        auto [bank1, bank2] = bus.cart_->get_prg_banks();

        auto find_prg_bank = [this] (const byte_t* rom) -> const PrgBank*
        {
            auto it = std::ranges::find_if(banks_, 
                [=](const PrgBank& prg) { return prg.rom_ <= rom && prg.rom_ + rom_size > rom; });

            if (it != banks_.end())
                return &*it;
            
            return nullptr;
        };

        return { find_prg_bank(bank1), find_prg_bank(bank2) };
    }

    return { nullptr, nullptr };
}