#include "cpu.h"

#include "bus.h"
#include "debugger.h"
#include "ops.h"
#include "ram.h"
#include "types.h"

#include <stdexcept>

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

std::string CallStats::report() const
{
    fmt::basic_memory_buffer<char, 4096> buf;

    auto it = fmt::format_to(buf.begin(), "OPS MIN MAX COUNT\n");
    it = fmt::format_to(it, "{:-<{}}\n\n", "-", 25);

    uint8_t ops = 0;
    for (Entry const& entry :  data_)
    {
        if (entry.count_ > 0)
            it = fmt::format_to(it, "{:02X}  {: <3} {: <3} {}\n", ops, entry.timing_.min, entry.timing_.max, entry.count_);

        ++ops;
    }

    return fmt::to_string(buf);
}

void CPU::step()
{
    for (;;)
    {
        if (state_ == kFetching)
        {
            state_ = step_fetch_();
            NES_ASSERT(idle_ticks_ > 0);
            break;
        }

        if (state_ == kIdle || state_ == kIRQ)
        {
            if (--idle_ticks_ > 0)
                break;
            
            state_ = (state_ == kIdle) ? kExecuting : kFetching;
        }

        if (state_ == kExecuting)
        {
            state_ = step_execute_();
            break;
        }
    }

    ++cycle_;
}

CPU::State CPU::step_fetch_()
{
    if (nmi_requested_ || irq_requested_)
    {
        if (nmi_requested_)
            nmi_();
        else if (!get_status_(kIntDisable))
            irq_();

        irq_requested_ = false;
        nmi_requested_ = false;

        if (idle_ticks_ > 0)
            return kIRQ;
    }

    instr_ = fetch_instr_(program_counter_);

#if 0
    log_(instr_);
#endif

    if (opcode_data(instr_.opcode).operation == kUKN)
    {
        program_counter_ = old_pc_;
        throw std::runtime_error(fmt::format(FMT_STRING("Unrecognized opcode {:02X}"), instr_.opcode));
    }

    auto& opdata = opcode_data(instr_.opcode);
    instr_.meta = opdata;

    idle_ticks_ = opdata.timing - 1;

    if (is_branch_(opdata.operation))
        idle_ticks_ += idle_ticks_from_branching_(instr_);
    else
        idle_ticks_ += idle_ticks_from_addressing_(instr_);

    instr_.time = idle_ticks_ + 1;
    
    Debugger::instance()->on_cpu_fetch(*this);

    return kIdle;
}

auto CPU::step_execute_() -> State
{
    old_pc_ = program_counter_;
    program_counter_ += opcode_data(instr_.opcode).get_size();
    exec_(instr_);

    auto& entry = stats_.data_[instr_.opcode];
    if (entry.count_ > 0)
    {
        auto mm = std::minmax({entry.timing_.min, entry.timing_.max, instr_.time});
        entry.timing_.min = mm.first;
        entry.timing_.max = mm.second;
    }
    else
    {
        entry.timing_.min = entry.timing_.max = instr_.time;
    }

    ++entry.count_;

    return kFetching;
}

uint8_t CPU::idle_ticks_from_branching_(Instr const& instr)
{
    bool success = false;

    switch (instr.meta.operation)
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

    
    address_t pc = program_counter_ + instr_.meta.get_size();
    address_t next_pc = pc + std::bit_cast<int8_t>(instr_.operands[0]);
    bool page_crossed = ((next_pc & 0xFF00) != (pc & 0xFF00));

    return 1 + (page_crossed ? 1 : 0);
}

uint8_t CPU::idle_ticks_from_addressing_(Instr const& instr)
{
    metadata const& meta = instr.meta;

    address_t addr = 0;
    address_t page_addr = 0;

    switch (meta.addressing)
    {
    case kAbsoluteX: 
        if (meta.timing != 4) return 0;
        page_addr = instr.to_addr();
        addr = indexed_abs_addr(page_addr, register_x_);
        break;
    case kAbsoluteY: 
        if (meta.timing != 4) return 0;
        page_addr = instr.to_addr();
        addr = indexed_abs_addr(page_addr, register_y_); 
        break;
    case kIndirectY: 
        if (meta.timing != 5) return 0;
        page_addr = indirect_pz_addr(0xFF & page_zero_addr(instr.to_addr())); 
        addr = page_addr + register_y_; 
        break;
    default:
        return 0;
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

    bool page_crossed = ((addr & 0xFF00) != (page_addr & 0xFF00));
    return page_crossed ? 1 : 0;
}

void CPU::reset()
{
    old_pc_ = 0x0000;
    program_counter_ = load_addr_(0xFFFC);
    //program_counter_ = 0x8000; //for CPU tests with nestest.nes

    accumulator_ = 0;
    register_x_ = 0;
    register_y_ = 0;
    status_ = 0x24;
    stack_pointer_ = 0xFD;

    irq_requested_ = false;
    nmi_requested_ = false;

    cycle_ = 0;
    idle_ticks_ = 7;
    state_ = kIRQ;
}

void CPU::pull_irq()
{
    irq_requested_ = true;
}

void CPU::pull_nmi()
{
    nmi_requested_ = true;
}

void CPU::log_(Instr instr)
{
    auto& opdata = opcode_data(instr.opcode);
    auto it = log_ring_[log_idx_].begin();
    
    it = fmt::format_to(it, "{:04x}    {:02x}", program_counter_, instr.opcode);

    if (opdata.get_size() > 1)
        it = fmt::format_to(it, "  {:02x}", instr.operands[0]);
    else
        it = fmt::format_to(it, "    ");

    if (opdata.get_size() > 2)
        it = fmt::format_to(it, "  {:02x}", instr.operands[1]);
    else
        it = fmt::format_to(it, "    ");

    it = fmt::format_to(it, " {} {:<27}", opdata.str, debug_addr_(opdata.addressing, instr.to_addr()));
    fmt::format_to(it, " A:{:02x} X:{:02x} Y:{:02x} P:{:02x} SP:{:02x}", accumulator_, register_x_, register_y_, status_, stack_pointer_);

    //fmt::print("{}\n", log_ring_[log_idx_].data());
    (++log_idx_) %= 64;
}

void CPU::irq_()
{
    store_stack_(program_counter_);

    set_status_(kBreak, false);
    set_status_(kIntDisable, true);
    store_stack_(status_);

    program_counter_ = load_addr_(0xFFFE);
    idle_ticks_ = 7;
}

void CPU::nmi_()
{
    store_stack_(program_counter_);

    set_status_(kBreak, false);
    set_status_(kIntDisable, true);
    store_stack_(status_);

    program_counter_ = load_addr_(0xFFFA);
    idle_ticks_ = 7;
}

auto CPU::fetch_instr_(address_t pc) -> Instr
{
    Instr i;
    auto& [op1, op2] = i.operands;

    i.opcode = bus_->read_cpu(program_counter_ + 0);
    op1 = bus_->read_cpu(program_counter_ + 1);
    op2 = bus_->read_cpu(program_counter_ + 2);

    return i;
}

void CPU::exec_(Instr instr)
{
    const byte_t opcode = instr.opcode;
    const address_t addr = instr.to_addr();

    const byte_t op = opcode_data(opcode).operation;
    const byte_t ad = opcode_data(opcode).addressing;

    auto to_absolute_addr = [=](address_t addr)
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
                throw std::runtime_error(fmt::format(FMT_STRING("Invalid addressing {:02X} for opcode {} [{:02X}]"), ad, opcode_data(opcode).str, opcode));
        }

        //return addr;
    };

    auto to_operand = [=](address_t addr)
    {
        if (ad == kImmediate)
            return immediate_addr(addr);
        else
            return load_(to_absolute_addr(addr));
    };

    auto call_mutating_operation = [=](void (CPU::*f)(byte_t&), address_t addr)
    {
        if (ad == kNone)
        {
            std::invoke(f, this, accumulator_);
        }
        else
        {
            addr = to_absolute_addr(addr);
            byte_t operand = load_(addr);
            std::invoke(f, this, operand);
            store_(addr, operand);
        }
    };

    // execute operation
    switch (op)
    {
    case kADC: adc_(to_operand(addr)); break; 
    case kAND: and_(to_operand(addr)); break;
    case kASL: call_mutating_operation(&CPU::asl_, addr); break; 

    case kBCC: bcc_(instr_.operands[0]); break;
    case kBCS: bcs_(instr_.operands[0]); break;
    case kBEQ: beq_(instr_.operands[0]); break; 
    case kBIT: bit_(to_absolute_addr(addr)); break; 
    case kBMI: bmi_(instr_.operands[0]); break;
    case kBNE: bne_(instr_.operands[0]); break;
    case kBPL: bpl_(instr_.operands[0]); break; 
    case kBRK: brk_(); break; 
    case kBVC: bvc_(instr_.operands[0]); break;
    case kBVS: bvs_(instr_.operands[0]); break;

    case kCLC: clc_(); break; 
    case kCLD: cld_(); break;
    case kCLI: cli_(); break;
    case kCLV: clv_(); break; 
    case kCMP: cmp_(to_operand(addr)); break;
    case kCPX: cpx_(to_operand(addr)); break;
    case kCPY: cpy_(to_operand(addr)); break; 

    case kDCP: dcp_(to_absolute_addr(addr)); break;
    case kDEC: dec_(to_absolute_addr(addr)); break; 
    case kDEX: dex_(); break; 
    case kDEY: dey_(); break;

    case kEOR: eor_(to_operand(addr)); break;

    case kINC: inc_(to_absolute_addr(addr)); break; 
    case kINX: inx_(); break;
    case kINY: iny_(); break; 
    case kISB: isb_(to_absolute_addr(addr)); break; 
    
    case kJMP: jmp_(to_absolute_addr(addr)); break; 
    case kJSR: jsr_(addr); break;

    case kLAX: lax_(to_absolute_addr(addr)); break; 
    case kLDA: lda_(to_operand(addr)); break; 
    case kLDX: ldx_(to_operand(addr)); break;
    case kLDY: ldy_(to_operand(addr)); break;
    case kLSR: call_mutating_operation(&CPU::lsr_, addr); break;

    case kNOP: nop_(); break;

    case kORA: ora_(to_operand(addr)); break;

    case kPHA: pha_(); break;
    case kPHP: php_(); break;
    case kPLA: pla_(); break; 
    case kPLP: plp_(); break; 

    case kRLA: rla_(to_absolute_addr(addr)); break; 

    case kROL: call_mutating_operation(&CPU::rol_, addr); break;
    case kROR: call_mutating_operation(&CPU::ror_, addr); break;
    case kRRA: rra_(to_absolute_addr(addr)); break; 
    case kRTI: rti_(); break; 
    case kRTS: rts_(); break; 

    case kSAX: sax_(to_absolute_addr(addr)); break; 
    case kSBC: sbc_(to_operand(addr)); break;
    case kSEC: sec_(); break; 
    case kSED: sed_(); break; 
    case kSEI: sei_(); break; 
    case kSLO: slo_(to_absolute_addr(addr)); break; 
    case kSRE: sre_(to_absolute_addr(addr)); break; 
    case kSTA: sta_(to_absolute_addr(addr)); break; 
    case kSTX: stx_(to_absolute_addr(addr)); break; 
    case kSTY: sty_(to_absolute_addr(addr)); break;

    case kTAX: tax_(); break; 
    case kTAY: tay_(); break; 
    case kTSX: tsx_(); break; 
    case kTXA: txa_(); break; 
    case kTXS: txs_(); break; 
    case kTYA: tya_(); break;

    default:
        throw std::runtime_error(fmt::format(FMT_STRING("Unimplemented operation {:02X} for opcode {} [{:02X}]"), op, opcode_data(opcode).str, opcode));
    }
}

inline void CPU::store_(address_t addr, byte_t operand)
{
    bus_->write_cpu(addr, operand);
}

inline void CPU::store_(address_t addr, address_t addr_value)
{
    bus_->write_cpu(addr + 1, static_cast<byte_t>(addr_value >> 8));
    bus_->write_cpu(addr, static_cast<byte_t>(0xFF & addr_value));
}

inline byte_t CPU::load_(address_t addr)
{
    return bus_->read_cpu(addr);
}

inline address_t CPU::load_addr_(address_t addr)
{
    address_t value;
    value = static_cast<address_t>(bus_->read_cpu(addr + 1)) << 8;
    value |= 0xFF & static_cast<address_t>(bus_->read_cpu(addr));
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
    return static_cast<byte_t>(addr & 0xFF);
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
    return static_cast<address_t>(bus_->read_cpu(addr_h)) << 8 | bus_->read_cpu(addr);
}

// $00
inline address_t CPU::page_zero_addr(address_t addr)
{
    return 0xFF & addr;
}

inline address_t CPU::indirect_pz_addr(byte_t addr)
{
    return static_cast<address_t>(bus_->read_cpu(static_cast<byte_t>(addr + 1))) << 8 | bus_->read_cpu(addr);
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
    const bool neg_a = accumulator_ & kNegative;
    const bool neg_op = operand & kNegative;

    const uint16_t sum = static_cast<uint16_t>(accumulator_) + operand + carry;
    accumulator_ = static_cast<byte_t>(sum);

    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
    set_status_(kCarry, sum > 0xFF);
    set_status_(kOverflow, (neg_a == neg_op) && neg_a != get_status_(kNegative));
}

void CPU::and_(byte_t operand)
{
    accumulator_ = accumulator_ & operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}

void CPU::asl_(byte_t& operand)
{
    set_status_(kCarry, operand & kNegative);
    operand <<= 1;
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
}

void CPU::bcc_(byte_t operand)
{
    if (!get_status_(kCarry))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::bcs_(byte_t operand)
{
    if (get_status_(kCarry))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::beq_(byte_t operand)
{
    if (get_status_(kZero))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::bit_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kNegative, operand & kNegative);
    set_status_(kOverflow, operand & kOverflow);
    operand &= accumulator_;
    set_status_(kZero, operand == 0);
    set_status_(kDummy, true);
}

void CPU::bmi_(byte_t operand)
{
    if (get_status_(kNegative))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::bne_(byte_t operand)
{
    if (!get_status_(kZero))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::bpl_(byte_t operand)
{
    if (!get_status_(kNegative))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::brk_()
{
    store_stack_(program_counter_);

    set_status_(kBreak, true);
    set_status_(kDummy, true);
    store_stack_(status_);
    set_status_(kIntDisable, true);
    
    program_counter_ = load_addr_(0xFFFE);
}

void CPU::bvc_(byte_t operand)
{
    if (!get_status_(kOverflow))
        program_counter_ += std::bit_cast<int8_t>(operand);
}

void CPU::bvs_(byte_t operand)
{
    if (get_status_(kOverflow))
        program_counter_ += std::bit_cast<int8_t>(operand);
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

void CPU::cpx_(byte_t operand)
{
    byte_t cmp = register_x_ - operand;
    set_status_(kZero, cmp == 0);
    set_status_(kNegative, cmp & kNegative);
    set_status_(kCarry, register_x_ >= operand);
}

void CPU::cpy_(byte_t operand)
{
    byte_t cmp = register_y_ - operand;
    set_status_(kZero, cmp == 0);
    set_status_(kNegative, cmp & kNegative);
    set_status_(kCarry, register_y_ >= operand);
}

void CPU::dcp_(address_t addr)
{
    dec_(addr);
    cmp_(load_(addr));
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
    sbc_(load_(addr));
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

void CPU::ldx_(byte_t operand)
{
    register_x_ = operand;
    set_status_(kZero, register_x_ == 0);
    set_status_(kNegative, register_x_ & kNegative);
}

void CPU::ldy_(byte_t operand)
{
    register_y_ = operand;
    set_status_(kZero, register_y_ == 0);
    set_status_(kNegative, register_y_ & kNegative);
}

void CPU::lsr_(byte_t& operand)
{
    set_status_(kCarry, operand & kCarry);
    operand >>= 1;
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

void CPU::pha_()
{
    store_stack_(accumulator_);
}

void CPU::php_()
{
    store_stack_(static_cast<byte_t>(status_ | 0b00110000));
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
    byte_t operand = load_(addr);
    rol_(operand);
    store_(addr, operand);
    and_(load_(addr));
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

void CPU::rra_(address_t addr)
{
    byte_t operand = load_(addr);
    ror_(operand);
    store_(addr, operand);
    adc_(load_(addr));
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
    adc_(~operand);
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
    byte_t operand = load_(addr);
    asl_(operand);
    store_(addr, operand);
    ora_(load_(addr));
}

void CPU::sre_(address_t addr)
{
    byte_t operand = load_(addr);
    lsr_(operand);
    store_(addr, operand);
    eor_(load_(addr));
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
