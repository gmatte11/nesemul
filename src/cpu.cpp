#include <cpu.h>

#include <types.h>
#include <ops.h>

namespace
{
    address_t immediate_addr(address_t addr)
    {
        return 0x0F & addr;
    }
}

void CPU::next()
{
    address_t addr;
    std::memcpy(memory_.data() + program_counter_, &addr, sizeof(address_t));
    exec_(ops_[0], addr);
}

void CPU::exec_(byte_t opcode, address_t addr)
{
    address_t program_counter = program_counter_;

    switch (opcode)
    {
        case kADC1: adc_(accumulator_, immediate_addr(addr));                     break;
        case kADC2: adc_(accumulator_, page_zero_addr(addr));                     break;
        case kADC3: adc_(accumulator_, indexed_pz_addr(addr, register_X));        break;
        case kADC4: adc_(accumulator_, absolute_addr(addr));                      break;
        case kADC5: adc_(accumulator_, indexed_absolute_addr(addr, register_X));  break;
        case kADC6: adc_(accumulator_, indexed_absolute_addr(addr, register_Y));  break;
        case kADC7: adc_(accumulator_, indirect_indexed_addr(addr, register_X));  break;
        case kADC8: adc_(accumulator_, indexed_indirect_addr(addr, register_Y));  break;

        case kAND1: and_(accumulator_, immediate_addr(addr));                     break;
        case kAND2: and_(accumulator_, page_zero_addr(addr));                     break;
        case kAND3: and_(accumulator_, indexed_pz_addr(addr, register_X));        break;
        case kAND4: and_(accumulator_, absolute_addr(addr));                      break;
        case kAND5: and_(accumulator_, indexed_absolute_addr(addr, register_X));  break;
        case kAND6: and_(accumulator_, indexed_absolute_addr(addr, register_Y));  break;
        case kAND7: and_(accumulator_, indirect_indexed_addr(addr, register_X));  break;
        case kAND8: and_(accumulator_, indexed_indirect_addr(addr, register_Y));  break;

        case kASL1: asl_(accumulator_);
        case kASL2: asl_(page_zero_addr(addr));
        case kASL3: asl_(indexed_pz_addr(addr,  register_X));
        case kASL4: asl_(absolute_addr(addr));
        case kASL5: asl_(indexed_absolute_addr(addr,  register_X));

        case kBCC : branch_if_carry(page_zero_addr(addr))
    }

    if (program_counter_ == program_counter)
    {
        program_counter_ += 1;
    }


}

inline void CPU::store_(address_t addr, byte_t value)
{
    memory_[addr] = value;
}

inline byte_t CPU::load_(address_t addr)
{
    return memory_[addr];
}

inline void CPU::set_status_(byte_t status_mask, bool set)
{
    if (set)
    {
        status_ |= status_mask;
    }
    else
    {
        status_ &= ~status_mask;
    }
}

inline bool CPU::get_status_(byte_t status_mask)
{
    return (status_ & status_mask) != 0;
}

void CPU::adc_(address_t addr)
{
    unsigned short result = static_cast<unsigned short>(accumulator_) + load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, result & kNegative);
    set_status_(kCarry, result & (1 << 8));
    set_status_(kOverflow, result & (1 << 9));
}

void CPU::and_(address_t addr)
{
    accumulator_ = accumulator_ & load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::asl_(address_t addr)
{
    byte_t val = load_(addr_);
    set_status_(kCarry, val & kNegative);
    store_(addr, val <<= 1);
    set_status_(kZero, val == 0);
    set_status_(kNegative, val & kNegative);
}

void CPU::bcc_(address_t addr)
{
    if (!get_status_(kCarry))
        program_counter_ = addr;
}

void CPU::bcs_(address_t addr)
{
    if (get_status_(kCarry))
        program_counter_ = addr;
}

void CPU::beq_(address_t addr)
{
    if (get_status_(kZero))
        program_counter_ = addr;
}

void CPU::bit_(address_t addr)
{
    set_status_(kZero, accumulator_ & load_(addr));
}

void CPU::bmi_(address_t addr)
{
    if (get_status_(kNegative))
        program_counter_ = addr;
}

void CPU::bne_(address_t addr)
{
    if (!get_status_(kZero))
        program_counter_ = addr;
}

void CPU::bpl_(address_t addr)
{
    if (!get_status_(kNegative))
        program_counter_ = addr;
}

void CPU::brk_(address_t addr)
{
    set_status_(kBreak, true);
    set_status_(kIntDisable, true);
}

void CPU::bvc_(address_t addr)
{
    if (!get_status_(kOverflow))
        program_counter_ = addr;
}

void CPU::bvs_(address_t addr)
{
    if (get_status_(kOverflow))
        program_counter_ = addr;
}

void CPU::clc_(address_t addr)
{
    set_status_(kCarry, false);
}

void CPU::cld_(address_t addr)
{
    set_status_(kDecimal, false);
}

void CPU::cli_(address_t addr)
{
    set_status_(kIntDisable, false);
}

void CPU::clv_(address_t addr)
{
    set_status_(kOverflow, false);
}

void CPU::cmp_(address_t addr)
{
    byte_t operand = load_(addr);
    if (accumulator_ <  operand) { set_status_(kNegative, true); set_status_(kZero | kCarry, false); }
    if (accumulator_ == operand) { set_status_(kNegative, false); set_status_(kZero | kCarry, true); }
    if (accumulator_ >  operand) { set_status_(kNegative | kZero, false); set_status_(kCarry, true); }
}

void CPU::cpx_(address_t addr)
{
    byte_t operand = load_(addr);
    if (register_x_ <  operand) { set_status_(kNegative, true); set_status_(kZero | kCarry, false); }
    if (register_x_ == operand) { set_status_(kNegative, false); set_status_(kZero | kCarry, true); }
    if (register_x_ >  operand) { set_status_(kNegative | kZero, false); set_status_(kCarry, true); }
}

void CPU::cpy_(address_t addr)
{
    byte_t operand = load_(addr);
    if (register_y_ <  operand) { set_status_(kNegative, true); set_status_(kZero | kCarry, false); }
    if (register_y_ == operand) { set_status_(kNegative, false); set_status_(kZero | kCarry, true); }
    if (register_y_ >  operand) { set_status_(kNegative | kZero, false); set_status_(kCarry, true); }
}

void CPU::dec_(address_t addr)
{
    byte_t operand = load_(addr) - 1;
    store_(addr, operand);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::dex_(address_t addr)
{
    --register_x_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::dey_(address_t addr)
{
    --register_y_;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::eor_(address_t addr)
{
    accumulator_ = accumulator_ ^ load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::inc_(address_t addr)
{
    byte_t operand = load_(addr) + 1;
    store_(addr, operand);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::inx_(address_t addr)
{
    ++register_x_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::iny_(address_t addr)
{
    ++register_y_;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::jmp_(address_t addr)
{
    program_counter_ = addr;
}

void CPU::jsr_(address_t addr)
{
    stack_pointer_ += sizeof(byte_t);
    store_(stack_pointer_, program_counter_)
    program_counter_ = addr;
}

void CPU::lda_(address_t addr)
{
    accumulator_ = load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::ldx_(address_t addr)
{
    register_x_ = load_(addr);
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::ldy_(address_t addr)
{
    register_y_ = load_(addr);
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::lsr_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kCarry);
    store_(addr, operand >>= 1);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::nop_(address_t addr)
{
}

void CPU::ora_(address_t addr)
{
    accumulator_ = accumulator_ | load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::pha_(address_t addr)
{
    stack_pointer_ += sizeof(byte_t);
    store_(stack_pointer_, accumulator_);
}

void CPU::php_(address_t addr)
{
    stack_pointer_ += sizeof(byte_t);
    store_(stack_pointer_, status_);
}

void CPU::pla_(address_t addr)
{
    accumulator_ = load_(stack_pointer_);
    stack_pointer_ -= sizeof(byte_t);
}

void CPU::plp_(address_t addr)
{
    status_ = load_(stack_pointer_);
    stack_pointer_ -= sizeof(byte_t);
}

void CPU::rol_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kNegative);
    operand <<= 1;
    if (get_status_(kCarry)) operand &= kCarry;
    store_(addr, operand);
}

void CPU::ror_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kCarry);
    operand >>= 1;
    if (get_status_(kCarry)) operand &= kNegative;
    store_(addr, operand);
}

void CPU::rti_(address_t addr)
{
}

void CPU::rts_(address_t addr)
{
    program_counter_ = load_(stack_pointer_);
    stack_pointer_ -= sizeof(byte_t);
}

void CPU::sbc_(address_t addr)
{
    unsigned short result = static_cast<unsigned short>(accumulator_) - load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, result & kNegative);
    set_status_(kCarry, result & (1 << 8));
    set_status_(kOverflow, result & (1 << 9));
}

void CPU::sec_(address_t addr)
{
    set_status_(kCarry, true);
}

void CPU::sei_(address_t addr)
{
    set_status_(kIntDisable, true);
}

void CPU::sta_(address_t addr)
{
    store_(addr, accumulator_);
}

void CPU::stx_(address_t addr)
{
    store_(addr, register_x_);
}

void CPU::sty_(address_t addr)
{
    store_(addr, register_y_);
}

void CPU::tax_(address_t addr)
{
    register_x_ = accumulator_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::tay_(address_t addr)
{
    register_y_ = accumulator_;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::tsx_(address_t addr)
{
    register_x_ = load_(stack_pointer_);
    stack_pointer_ -= sizeof(byte_t);
}

void CPU::txa_(address_t addr)
{
    accumulator_ = register_x_;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::txs_(address_t addr)
{
    stack_pointer_ += sizeof(byte_t);
    store_(stack_pointer_, register_x_);
}

void CPU::tya_(address_t addr)
{
    accumulator_ = register_y_;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}
