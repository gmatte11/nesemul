#include "cpu.h"

#include "types.h"
#include "ops.h"
#include "bus.h"
#include "ram.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>

#include <cstring>

#include <fmt/format.h>

using namespace ops;

std::string _str(byte_t operand)
{
    return fmt::format("{:02x}", operand);
}

std::string _str(address_t addr)
{
    return fmt::format("{:04x}", addr);
}

void CPU::step()
{
    if (idle_ticks_ == 0 && int_.first)
    {
        if (int_.second)
            nmi_();
        else
            irq_();

        int_ = {false, false};
    }

    if (idle_ticks_ == 0)
    {
        instr_.opcode = bus_->read(program_counter_ + 0);
        instr_.op1 = bus_->read(program_counter_ + 1);
        instr_.op2 = bus_->read(program_counter_ + 2);

#if 0
        log_(data.opcode, addr);
#endif
        if (opcode_data(instr_.opcode).operation == kUKN)
        {
            program_counter_ = old_pc_;
            throw std::runtime_error(fmt::format("Unrecognized opcode {:02x}", instr_.opcode));
        }

        idle_ticks_ = opcode_data(instr_.opcode).timing;

        if (is_branch_(instr_.opcode))
            idle_ticks_ += idle_ticks_from_branching_(instr_.opcode, instr_.to_addr());
        else
            idle_ticks_ += idle_ticks_from_addressing_(opcode_data(instr_.opcode).addressing, instr_.to_addr());

        old_pc_ = program_counter_;
        program_counter_ += opcode_data(instr_.opcode).size;
        exec_(instr_.opcode, instr_.to_addr());
    }

    if (idle_ticks_ > 0)
        --idle_ticks_;

    ++cycle_;
}

int CPU::idle_ticks_from_branching_(byte_t opcode, address_t addr)
{
    bool success = false;

    switch (opcode)
    {
    case kBCC: success = !get_status_(kCarry); break;
    case kBCS: success = get_status_(kCarry); break;
    case kBEQ: success = get_status_(kZero); break;
    case kBMI: success = get_status_(kNegative); break;
    case kBNE: success = !get_status_(kZero); break;
    case kBPL: success = !get_status_(kNegative); break;
    case kBVC: success = !get_status_(kOverflow); break;
    case kBVS: success = get_status_(kOverflow); break;
    default: return 0;
    }

    if (!success)
        return 0;

    address_t next_pc = program_counter_ + static_cast<int8_t>(0xFF & addr);
    bool page_crossed = ((next_pc & 0xFF00) != (program_counter_ & 0xFF00));

    return 1 + (page_crossed ? 1 : 0);
}

int CPU::idle_ticks_from_addressing_(byte_t addr_mode, address_t operands)
{
    address_t addr = 0;

    switch (addr_mode)
    {
    case kAbsoluteX: addr = indexed_abs_addr(operands, register_x_); break;
    case kAbsoluteY: addr = indexed_abs_addr(operands, register_y_); break;
    case kIndirectY: 
        operands = indirect_pz_addr(0xFF & page_zero_addr(operands)); 
        addr = operands + register_y_;
        break;
    default: addr = operands;
    }

    // TODO
    /*switch (instr_.opcode)
    {
    case kASL5:
    case kDEC4:
    case kINC4:
    case kLSR5:
    case kROL5:
    case kROR5:
    case kSTA5:
    case kSTA6:
    case kSTA8:
        return 0;
    }*/

    bool page_crossed = ((addr & 0xFF00) != (operands & 0xFF00));
    return page_crossed ? 1 : 0;
}

void CPU::reset()
{
    old_pc_ = program_counter_;
    program_counter_ = load_addr_(0xFFFC);
    //program_counter_ = 0x8000; //for CPU tests with nestest.nes

    instr_.opcode = 0xFF;

    accumulator_ = 0;
    register_x_ = 0;
    register_y_ = 0;
    status_ = 0x24;
    stack_pointer_ = 0xFD;

    cycle_ = 0;
    idle_ticks_ = 8;
}

void CPU::interrupt(bool nmi /*= false*/)
{
    int_ = {true, nmi};
}

void CPU::log_(byte_t opcode, address_t addr)
{
    auto& opdata = opcode_data(opcode);
    auto& it = log_ring_[log_idx_].begin();
    
    it = fmt::format_to(it, "{:04x}    {:02x}", program_counter_, opcode);

    if (opdata.size > 1)
        it = fmt::format_to(it, "  {:02x}", addr & 0xFF);
    else
        it = fmt::format_to(it, "    ");

    if (opdata.size > 2)
        it = fmt::format_to(it, "  {:02x}", (addr >> 4) & 0xFF);
    else
        it = fmt::format_to(it, "    ");

    it = fmt::format_to(it, " {} {:<27}", opdata.str, debug_addr_(opcode_data(opcode).addressing, addr));
    fmt::format_to(it, " A:{:02x} X:{:02x} Y:{:02x} P:{:02x} SP:{:02x}", accumulator_, register_x_, register_y_, status_, stack_pointer_);

    //fmt::print("{}\n", log_ring_[log_idx_].data());
    (++log_idx_) %= 64;
}

void CPU::irq_()
{
    if (get_status_(kIntDisable))
        return;

    store_stack_(program_counter_);

    set_status_(kBreak, false);
    set_status_(kIntDisable, true);
    store_stack_(status_);

    address_t vector = 0xFFFE;
    program_counter_ = load_addr_(vector);
    idle_ticks_ = 7;
}

void CPU::nmi_()
{
    store_stack_(program_counter_);

    set_status_(kBreak, false);
    set_status_(kIntDisable, true);
    store_stack_(status_);

    address_t vector = 0xFFFA;
    program_counter_ = load_addr_(vector);
    idle_ticks_ = 8;
}

void CPU::exec_(byte_t opcode, address_t addr)
{
    const byte_t op = opcode_data(opcode).operation;
    const byte_t ad = opcode_data(opcode).addressing;

    auto addressing = [=] 
    {  
        switch (ad)
        {
            case kZeroPage: return page_zero_addr(addr);
            case kZeroPageX: return indexed_pz_addr(addr, register_x_);
            case kZeroPageY: return indexed_pz_addr(addr, register_y_);
            case kAbsolute: return absolute_addr(addr);
            case kAbsoluteX: return indexed_abs_addr(addr, register_x_);
            case kAbsoluteY: return indexed_abs_addr(addr, register_y_);
            case kIndirect: return indirect_addr(addr);
            case kIndirectX: return indexed_indirect_addr(addr, register_x_);
            case kIndirectY : return indirect_indexed_addr(addr, register_y_);
            
            default: 
                throw std::runtime_error(fmt::format("Invalid addressing {:02x} for opcode {:02x}", ad, opcode));
        }

        return addr;
    };


    // execute operation
    switch (op)
    {
    case kADC:
        if (ad == kImmediate)
            adc_(immediate_addr(addr));
        else
            adc_(addressing());
        break; 
    
    case kAND:
        if (ad == kImmediate)
            and_(immediate_addr(addr));
        else
            and_(addressing());
        break; 

    case kASL:
        if (ad == kNone)
            asl_();
        else
            asl_(addressing());
        break;

    case kBCC:
        bcc_(addr);
        break;

    case kBCS:
        bcs_(addr);
        break;

    case kBEQ:
        beq_(addr);
        break;

    case kBIT:
        bit_(addressing());
        break;

    case kBMI:
        bmi_(addr);
        break;

    case kBNE:
        bne_(addr);
        break;

    case kBPL:
        bpl_(addr);
        break;

    case kBRK:
        brk_();
        break;

    case kBVC:
        bvc_(addr);
        break;

    case kBVS:
        bvs_(addr);
        break;

    case kCLC:
        clc_();
        break;

    case kCLD:
        cld_();
        break;

    case kCLI:
        cli_();
        break;

    case kCLV:
        clv_();
        break;

    case kCMP:
        if (ad == kImmediate)
            cmp_(immediate_addr(addr));
        else
            cmp_(addressing());
        break;

    case kCPX:
        if (ad == kImmediate)
            cpx_(immediate_addr(addr));
        else
            cpx_(addressing());
        break;

    case kCPY:
        if (ad == kImmediate)
            cpy_(immediate_addr(addr));
        else
            cpy_(addressing());
        break;

    case kDCP:
        dcp_(addressing());
        break;

    case kDEC:
        dec_(addressing());
        break;

    case kDEX:
        dex_();
        break;

    case kDEY:
        dey_();
        break;

    case kEOR:
        if (ad == kImmediate)
            eor_(immediate_addr(addr));
        else
            eor_(addressing());
        break;

    case kINC:
        inc_(addressing());
        break;

    case kINX:
        inx_();
        break;

    case kINY:
        iny_();
        break;

    case kISB:
        isb_(addressing());
        break;

    case kJMP:
        jmp_(addressing());
        break;

    case kJSR:
        jsr_(absolute_addr(addr));
        break; // addr: $aaaa

    case kLAX:
        lax_(addressing());
        break;

    case kLDA:
        if (ad == kImmediate)
            lda_(immediate_addr(addr));
        else
            lda_(addressing());
        break;

    case kLDX:
        if (ad == kImmediate)
            ldx_(immediate_addr(addr));
        else
            ldx_(addressing());
        break;

    case kLDY:
        if (ad == kImmediate)
            ldy_(immediate_addr(addr));
        else
            ldy_(addressing());
        break;

    case kLSR:
        if (ad == kNone)
            lsr_();
        else
            lsr_(addressing());
        break;

    case kNOP:
        nop_();
        break;

    case kORA:
        if (ad == kImmediate)
            ora_(immediate_addr(addr));
        else
            ora_(addressing());
        break;

    case kPHA:
        pha_();
        break;

    case kPHP:
        php_();
        break;

    case kPLA:
        pla_();
        break;

    case kPLP:
        plp_();
        break;

    case kRLA:
        rla_(addressing());
        break;

    case kROL:
        if (ad == kNone)
            rol_(accumulator_);
        else
            rol_(addressing());
        break;

    case kROR:
        if (ad == kNone)
            ror_(accumulator_);
        else
            ror_(addressing());
        break;

    case kRRA:
        rra_(addressing());
        break;

    case kRTI:
        rti_();
        break;

    case kRTS:
        rts_();
        break;

    case kSAX:
        sax_(addressing());
        break;

    case kSBC:
        if (ad == kImmediate)
            sbc_(immediate_addr(addr));
        else
            sbc_(addressing());
        break;

    case kSEC:
        sec_();
        break;

    case kSED:
        sed_();
        break;

    case kSEI:
        sei_();
        break;

    case kSLO:
        slo_(addressing());
        break;

    case kSRE:
        sre_(addressing());
        break;

    case kSTA:
        sta_(addressing());
        break;

    case kSTX:
        stx_(addressing());
        break;

    case kSTY:
        sty_(addressing());
        break;

    case kTAX:
        tax_();
        break;

    case kTAY:
        tay_();
        break;

    case kTSX:
        tsx_();
        break;

    case kTXA:
        txa_();
        break;

    case kTXS:
        txs_();
        break;

    case kTYA:
        tya_();
        break;

    default:
        throw std::runtime_error(fmt::format("Unimplemented operation {:02x} for opcode {:02x}", op, opcode));
    }
}

inline void CPU::store_(address_t addr, byte_t operand)
{
    bus_->write(addr, operand);
}

inline void CPU::store_(address_t addr, address_t addr_value)
{
    bus_->write(addr + 1, static_cast<byte_t>(addr_value >> 8));
    bus_->write(addr, static_cast<byte_t>(0xFF & addr_value));
}

inline byte_t CPU::load_(address_t addr)
{
    return bus_->read(addr);
}

inline address_t CPU::load_addr_(address_t addr)
{
    address_t value;
    value = static_cast<address_t>(bus_->read(addr + 1)) << 8;
    value |= 0xFF & static_cast<address_t>(bus_->read(addr));
    return value;
}

inline void CPU::store_stack_(byte_t operand)
{
    store_(0x0100 | stack_pointer_, operand);
    --stack_pointer_;
}

inline void CPU::store_stack_(address_t addr)
{
    --stack_pointer_;
    store_(0x0100 | stack_pointer_, addr);
    --stack_pointer_;
}

inline void CPU::load_stack_(byte_t& dest)
{
    ++stack_pointer_;
    dest = load_(0x0100 | stack_pointer_);
}

inline void CPU::load_stack_(address_t& dest)
{
    ++stack_pointer_;
    dest = load_addr_(0x0100 | stack_pointer_);
    ++stack_pointer_;
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

// #$00
inline byte_t CPU::immediate_addr(address_t addr)
{
    return 0xFF & addr;
}

// $0000
inline address_t CPU::absolute_addr(address_t addr)
{
    return addr;
}

// $0000,X
inline address_t CPU::indexed_abs_addr(address_t addr, byte_t index)
{
    return addr + index;
}

// $(0000)
inline address_t CPU::indirect_addr(address_t addr)
{
    address_t addr_h = (0xFF00 & addr) | (0xFF & addr + 1);
    return static_cast<address_t>(bus_->read(addr_h)) << 8 | bus_->read(addr);
}

// $00
inline address_t CPU::page_zero_addr(address_t addr)
{
    return 0xFF & addr;
}

inline address_t CPU::indirect_pz_addr(byte_t addr)
{
    return static_cast<address_t>(bus_->read(static_cast<byte_t>(addr + 1))) << 8 | bus_->read(addr);
}

// $(00)
inline address_t CPU::indexed_pz_addr(address_t addr, byte_t index)
{
    return static_cast<byte_t>(static_cast<byte_t>(0xFF & addr) + index);
}

// $(00,X)
inline address_t CPU::indexed_indirect_addr(address_t addr, byte_t index)
{
    return indirect_pz_addr(0xFF & indexed_pz_addr(addr, index));
}

// $(00),Y
inline address_t CPU::indirect_indexed_addr(address_t addr, byte_t index)
{
    return indirect_pz_addr(0xFF & page_zero_addr(addr)) + index;
}

std::string CPU::debug_addr_(byte_t type, address_t addr)
{
    byte_t addr_l = static_cast<byte_t>(0xFF & addr);

    switch (type)
    {
    case ops::kImmediate:
        return fmt::format("#${:02x}", addr_l);

    case ops::kZeroPage:
        return fmt::format("${:02x}", addr_l);

    case ops::kZeroPageX:
        addr = indexed_pz_addr(addr, register_x_);
        return fmt::format("${:02x},X  @ {:04x}", addr_l, addr);

    case ops::kAbsolute:
        return fmt::format("${:04x}", addr);

    case ops::kIndirect:
        return fmt::format("(${:04x})", addr);

    case ops::kIndirectX:
        addr = indexed_indirect_addr(addr, register_x_);
        return fmt::format("(${:02x},X) @ {:04x}", addr_l, addr);

    default:
        break;
    }

    return std::string{};
}

bool CPU::is_branch_(byte_t opcode) const
{
    switch (opcode)
    {
    case kBCC:
    case kBCS:
    case kBEQ:
    case kBMI:
    case kBNE:
    case kBPL:
    case kBVC:
    case kBVS:
        return true;

    default:
        return false;
    }
}

void CPU::adc_(byte_t operand)
{
    byte_t carry = (get_status_(kCarry)) ? 0x01 : 0x00;
    bool neg_a = accumulator_ & kNegative;
    bool neg_op = operand & kNegative;

    accumulator_ += operand + carry;

    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
    set_status_(kCarry, accumulator_ < operand);
    set_status_(kOverflow, (neg_a == neg_op) && neg_a != get_status_(kNegative));
}

void CPU::adc_(address_t addr)
{
    adc_(load_(addr));
}

void CPU::and_(byte_t operand)
{
    accumulator_ = accumulator_ & operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::and_(address_t addr)
{
    and_(load_(addr));
}

void CPU::asl_()
{
    set_status_(kCarry, accumulator_ & kNegative);
    accumulator_ <<= 1;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::asl_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kNegative);
    store_(addr, operand <<= 1);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::bcc_(address_t addr)
{
    if (!get_status_(kCarry))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::bcs_(address_t addr)
{
    if (get_status_(kCarry))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::beq_(address_t addr)
{
    if (get_status_(kZero))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::bit_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kNegative, operand & kNegative);
    set_status_(kOverflow, operand & kOverflow);
    operand &= accumulator_;
    set_status_(kZero, operand == 0);
    set_status_(0x20, true); // always 1 flag
}

void CPU::bmi_(address_t addr)
{
    if (get_status_(kNegative))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::bne_(address_t addr)
{
    if (!get_status_(kZero))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::bpl_(address_t addr)
{
    if (!get_status_(kNegative))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::brk_()
{
    set_status_(kBreak, true);
    set_status_(kIntDisable, true);
    store_stack_(program_counter_);
    store_stack_(status_);
    set_status_(kBreak, false);
    program_counter_ = load_addr_(0xFFFE);
}

void CPU::bvc_(address_t addr)
{
    if (!get_status_(kOverflow))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::bvs_(address_t addr)
{
    if (get_status_(kOverflow))
        program_counter_ += static_cast<int8_t>(0xFF & addr);
}

void CPU::clc_()
{
    set_status_(kCarry, false);
}

void CPU::cld_()
{
    set_status_(kDecimal, false);
}

void CPU::cli_()
{
    set_status_(kIntDisable, false);
}

void CPU::clv_()
{
    set_status_(kOverflow, false);
}

void CPU::cmp_(byte_t operand)
{
    byte_t cmp = accumulator_ - operand;
    set_status_(kZero, cmp == 0);
    set_status_(kNegative, cmp & kNegative);
    set_status_(kCarry, accumulator_ >= operand);
}

void CPU::cmp_(address_t addr)
{
    cmp_(load_(addr));
}

void CPU::cpx_(byte_t operand)
{
    byte_t cmp = register_x_ - operand;
    set_status_(kZero, cmp == 0);
    set_status_(kNegative, cmp & kNegative);
    set_status_(kCarry, register_x_ >= operand);
}

void CPU::cpx_(address_t addr)
{
    cpx_(load_(addr));
}

void CPU::cpy_(byte_t operand)
{
    byte_t cmp = register_y_ - operand;
    set_status_(kZero, cmp == 0);
    set_status_(kNegative, cmp & kNegative);
    set_status_(kCarry, register_y_ >= operand);
}

void CPU::cpy_(address_t addr)
{
    cpy_(load_(addr));
}

void CPU::dcp_(address_t addr)
{
    dec_(addr);
    cmp_(addr);
}

void CPU::dec_(address_t addr)
{
    byte_t operand = load_(addr) - 1;
    store_(addr, operand);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::dex_()
{
    --register_x_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::dey_()
{
    --register_y_;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::eor_(byte_t operand)
{
    accumulator_ = accumulator_ ^ operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::eor_(address_t addr)
{
    eor_(load_(addr));
}

void CPU::inc_(address_t addr)
{
    byte_t operand = load_(addr) + 1;
    store_(addr, operand);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::inx_()
{
    ++register_x_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::iny_()
{
    ++register_y_;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::isb_(address_t addr)
{
    inc_(addr);
    sbc_(addr);
}

void CPU::jmp_(address_t addr)
{
    program_counter_ = addr;
}

void CPU::jsr_(address_t addr)
{
    store_stack_(static_cast<address_t>(program_counter_ - 1));
    program_counter_ = addr;
}

void CPU::lax_(address_t addr)
{
    register_x_ = accumulator_ = load_(addr);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::lda_(byte_t operand)
{
    accumulator_ = operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::lda_(address_t addr)
{
    lda_(load_(addr));
}

void CPU::ldx_(byte_t operand)
{
    register_x_ = operand;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::ldx_(address_t addr)
{
    ldx_(load_(addr));
}

void CPU::ldy_(byte_t operand)
{
    register_y_ = operand;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::ldy_(address_t addr)
{
    ldy_(load_(addr));
}

void CPU::lsr_()
{
    set_status_(kCarry, accumulator_ & kCarry);
    accumulator_ >>= 1;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::lsr_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kCarry);
    store_(addr, operand >>= 1);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::nop_()
{
}

void CPU::ora_(byte_t operand)
{
    accumulator_ = accumulator_ | operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::ora_(address_t addr)
{
    ora_(load_(addr));
}

void CPU::pha_()
{
    store_stack_(accumulator_);
}

void CPU::php_()
{
    store_stack_(status_);
}

void CPU::pla_()
{
    load_stack_(accumulator_);
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::plp_()
{
    load_stack_(status_);
    set_status_(0x20, true); // always 1 flag
}

void CPU::rla_(address_t addr)
{
    rol_(addr);
    and_(addr);
}

void CPU::rol_(byte_t& operand)
{
    byte_t carry = operand & kNegative;
    operand <<= 1;
    if (get_status_(kCarry))
        operand |= kCarry;
    set_status_(kCarry, carry != 0);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::rol_(address_t addr)
{
    byte_t operand = load_(addr);
    rol_(operand);
    store_(addr, operand);
}

void CPU::ror_(byte_t& operand)
{
    byte_t carry = operand & kCarry;
    operand >>= 1;
    if (get_status_(kCarry))
        operand |= kNegative;
    set_status_(kCarry, carry != 0);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::ror_(address_t addr)
{
    byte_t operand = load_(addr);
    ror_(operand);
    store_(addr, operand);
}

void CPU::rra_(address_t addr)
{
    ror_(addr);
    adc_(addr);
}

void CPU::rti_()
{
    load_stack_(status_);
    load_stack_(program_counter_);
    //set_status_(0x20, true); // always 1 flag
}

void CPU::rts_()
{
    load_stack_(program_counter_);
    ++program_counter_;
}

void CPU::sax_(address_t addr)
{
    store_(addr, static_cast<byte_t>(accumulator_ & register_x_));
}

void CPU::sbc_(byte_t operand)
{
    byte_t carry = (!get_status_(kCarry)) ? 0x01 : 0x00;
    bool neg_a = accumulator_ & kNegative;
    bool neg_op = operand & kNegative;

    accumulator_ = accumulator_ - operand - carry;

    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
    set_status_(kCarry, accumulator_ < (operand | kNegative));
    set_status_(kOverflow, (neg_a == !neg_op) && neg_a != get_status_(kNegative));
}

void CPU::sbc_(address_t addr)
{
    sbc_(load_(addr));
}

void CPU::sec_()
{
    set_status_(kCarry, true);
}

void CPU::sed_()
{
    set_status_(kDecimal, true);
}

void CPU::sei_()
{
    set_status_(kIntDisable, true);
}

void CPU::slo_(address_t addr)
{
    asl_(addr);
    ora_(addr);
}

void CPU::sre_(address_t addr)
{
    lsr_(addr);
    eor_(addr);
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

void CPU::tax_()
{
    register_x_ = accumulator_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::tay_()
{
    register_y_ = accumulator_;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::tsx_()
{
    register_x_ = stack_pointer_;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::txa_()
{
    accumulator_ = register_x_;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::txs_()
{
    stack_pointer_ = register_x_;
}

void CPU::tya_()
{
    accumulator_ = register_y_;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}
