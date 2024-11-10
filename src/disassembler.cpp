#include "disassembler.h"

#include "ops.h"
#include "bus.h"
#include "cartridge.h"
#include "utils.h"

#include <algorithm>
#include <ranges>


void Disassembler::load(const BUS& bus)
{
    cart_ = bus.cart_;

    banks_.clear();

    if (cart_ == nullptr)
        return;

    auto rom_banks = cart_->get_prg_banks();
    const int count = static_cast<int>(std::size(rom_banks));

    banks_.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        PrgBank& bank = banks_.emplace_back();
        bank.rom_ = &*std::begin(rom_banks[i]);
        bank.rom_bank_idx_ = i;

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
    bank.rom_size_ = static_cast<int>(size);
}

void Disassembler::render(StringBuilder& sb, address_t addr, int offset) const
{
    if (cart_ == nullptr)
        return;

    auto [rom, rom_addr] = cart_->get_cpu_mapped_bank(addr);

    auto it_banks = std::find_if(banks_.begin(), banks_.end(), [=](PrgBank const& bank) { return bank.rom_ == rom.data(); });
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

int Disassembler::get_ops_count(const byte_t* bank_mem, int size)
{
    for (const PrgBank& prg_bank : banks_)
    {
        if (bank_mem >= prg_bank.rom_ && bank_mem < prg_bank.rom_ + prg_bank.rom_size_)
        {
            NES_ASSERT(bank_mem + size <= prg_bank.rom_ + prg_bank.rom_size_);

            auto it = prg_bank.ops_.begin();

            const byte_t* ptr = prg_bank.rom_;
            while (ptr < bank_mem)
            {
                const int width = ops::opcode_data(it->opcode_).get_size();
                ptr += width;
            }

            int count = 0;

            while (ptr < bank_mem + size)
            {
                const int width = ops::opcode_data(it->opcode_).get_size();
                ptr += width;
                ++count;
            }

            return count;
        }
    }

    return -1;
}