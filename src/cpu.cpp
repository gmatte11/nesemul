#include <cpu.h>

#include <types.h>
#include <ops.h>

#include <stdexcept>
#include <sstream>
#include <iomanip>

#include <cstring>

#include <iostream>

using namespace ops;

namespace
{
    typedef int8_t sbyte_t;
    inline sbyte_t sign(byte_t b)
    {
        return *reinterpret_cast<sbyte_t *>(&b);
    }

    std::string _str(byte_t operand)
    {
        std::ostringstream oss;

        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)operand;

        return oss.str();
    }

    std::string _str(address_t addr)
    {
        std::ostringstream oss;

        oss << std::hex
            << std::setw(4)
            << std::setfill('0')
            << addr;

        return oss.str();
    }
}

void CPU::next()
{
    struct
    {
        byte_t opcode;
        byte_t addr_l;
        byte_t addr_h;
    } data;

    std::memcpy(&data, memory_.data() + program_counter_, sizeof(data));

    address_t addr = static_cast<address_t>(data.addr_h) << 8 | data.addr_l;

    std::cout
        << std::hex << std::setfill('0')
        << std::setw(4) << program_counter_ << "  "
        << "  " << std::setw(2) << _str(data.opcode);

    if (opcode_data(data.opcode).size > 1)
        std::cout << "  " << std::setw(2) << _str(data.addr_l);
    else
        std::cout << "    ";

    if (opcode_data(data.opcode).size > 2)
        std::cout << "  " << std::setw(2) << _str(data.addr_h);
    else
        std::cout << "    ";

    std::cout << " " << opcode_data(data.opcode).str;

    std::cout << " "
        << std::setfill(' ') << std::left << std::setw(27)
        << debug_addr_(opcode_data(data.opcode).addressing, addr);

    std::cout
        << " A:" << std::setw(2) << _str(accumulator_)
        << " X:" << std::setw(2) << _str(register_x_)
        << " Y:" << std::setw(2) << _str(register_y_)
        << " P:" << std::setw(2) << _str(status_)
        << " SP:" << std::setw(2) << _str(stack_pointer_)
        << '\n';

    if (opcode_data(data.opcode).size == 0)
    {
        std::ostringstream oss;
        oss << std::hex << "Unrecognized opcode " << _str(data.opcode);
        throw std::runtime_error(oss.str());
    }

    program_counter_ += opcode_data(data.opcode).size;

    exec_(data.opcode, addr);

    static int count = 0;
    if (++count > 9000)
    {
        throw std::runtime_error("too many operations");
    }
}

void CPU::exec_(byte_t opcode, address_t addr)
{
    // execute operation
    switch (opcode)
    {
        case kADC1: adc_(immediate_addr(addr));                     break; // addr: #aa
        case kADC2: adc_(page_zero_addr(addr));                     break; // addr: $aa
        case kADC3: adc_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kADC4: adc_(absolute_addr(addr));                      break; // addr: $aaaa
        case kADC5: adc_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kADC6: adc_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kADC7: adc_(indexed_indirect_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kADC8: adc_(indirect_indexed_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kAND1: and_(immediate_addr(addr));                     break; // addr: #aa
        case kAND2: and_(page_zero_addr(addr));                     break; // addr: $aa
        case kAND3: and_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kAND4: and_(absolute_addr(addr));                      break; // addr: $aaaa
        case kAND5: and_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kAND6: and_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kAND7: and_(indexed_indirect_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kAND8: and_(indirect_indexed_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kASL1: asl_();                                         break; // addr: A
        case kASL2: asl_(page_zero_addr(addr));                     break; // addr: $aa
        case kASL3: asl_(indexed_pz_addr(addr,  register_x_));       break; // addr: $aa,X
        case kASL4: asl_(absolute_addr(addr));                      break; // addr: $aaaa
        case kASL5: asl_(indexed_abs_addr(addr,  register_x_));      break; // addr: $aaaa,X

        case kBCC: bcc_(page_zero_addr(addr)); break; // addr: $aa

        case kBCS: bcs_(page_zero_addr(addr)); break; // addr: $aa

        case kBEQ: beq_(page_zero_addr(addr)); break; // addr: $aa

        case kBIT1: bit_(page_zero_addr(addr)); break; // addr: $aa
        case kBIT2: bit_(absolute_addr(addr));  break; // addr: $aaaa

        case kBMI: bmi_(page_zero_addr(addr)); break; // addr: $aa

        case kBNE: bne_(page_zero_addr(addr)); break; // addr: $aa

        case kBPL: bpl_(page_zero_addr(addr)); break; // addr: $aa

        case kBRK: brk_();  break;

        case kBVC: bvc_(page_zero_addr(addr)); break; // addr: $aa

        case kBVS: bvs_(page_zero_addr(addr)); break; // addr: $aa

        case kCLC: clc_();  break;

        case kCLD: cld_();  break;

        case kCLI: cli_();  break;

        case kCLV: clv_();  break;

        case kCMP1: cmp_(immediate_addr(addr));                     break; // addr: #aa
        case kCMP2: cmp_(page_zero_addr(addr));                     break; // addr: $aa
        case kCMP3: cmp_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kCMP4: cmp_(absolute_addr(addr));                      break; // addr: $aaaa
        case kCMP5: cmp_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kCMP6: cmp_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kCMP7: cmp_(indexed_indirect_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kCMP8: cmp_(indirect_indexed_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kCPX1: cpx_(immediate_addr(addr)); break; // addr: #aa
        case kCPX2: cpx_(page_zero_addr(addr)); break; // addr: $aa
        case kCPX3: cpx_(absolute_addr(addr));  break; // addr: $aaaa

        case kCPY1: cpy_(immediate_addr(addr)); break; // addr: #aa
        case kCPY2: cpy_(page_zero_addr(addr)); break; // addr: $aa
        case kCPY3: cpy_(absolute_addr(addr));  break; // addr: $aaaa

        case kDEC1: dec_(page_zero_addr(addr));                 break; // addr: $aa
        case kDEC2: dec_(indexed_pz_addr(addr, register_x_));    break; // addr: $aa,X
        case kDEC3: dec_(absolute_addr(addr));                  break; // addr: $aaaa
        case kDEC4: dec_(indexed_abs_addr(addr, register_x_));   break; // addr: $aaaa,X

        case kDEX: dex_();  break;

        case kDEY: dey_();  break;

        case kEOR1: eor_(immediate_addr(addr));                     break; // addr: #aa
        case kEOR2: eor_(page_zero_addr(addr));                     break; // addr: $aa
        case kEOR3: eor_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kEOR4: eor_(absolute_addr(addr));                      break; // addr: $aaaa
        case kEOR5: eor_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kEOR6: eor_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kEOR7: eor_(indexed_indirect_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kEOR8: eor_(indirect_indexed_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kINC1: inc_(page_zero_addr(addr));                 break; // addr: $aa
        case kINC2: inc_(indexed_pz_addr(addr, register_x_));    break; // addr: $aa,X
        case kINC3: inc_(absolute_addr(addr));                  break; // addr: $aaaa
        case kINC4: inc_(indexed_abs_addr(addr, register_x_));   break; // addr: $aaaa,X

        case kINX: inx_();  break;

        case kINY: iny_();  break;

        case kJMP1: jmp_(absolute_addr(addr)); break; // addr: $aaaa
        case kJMP2: jmp_(indirect_addr(addr)); break; // addr: ($aaaa)

        case kJSR: jsr_(absolute_addr(addr));   break; // addr: $aaaa

        case kLAX1: lax_(indexed_indirect_addr(addr, register_x_)); break; // addr: ($aa,X)
        case kLAX2: lax_(page_zero_addr(addr));                     break; // addr: $aa
        case kLAX3: lax_(absolute_addr(addr));                      break; // addr: $aaaa
        case kLAX4: lax_(indirect_indexed_addr(addr, register_y_)); break; // addr: ($aa),Y
        case kLAX5: lax_(indexed_pz_addr(addr, register_y_));       break; // addr: $aa,Y
        case kLAX6: lax_(indexed_abs_addr(addr, register_y_));      break; // addr: $aaaa,Y


        case kLDA1: lda_(immediate_addr(addr));                      break; // addr: #aa
        case kLDA2: lda_(page_zero_addr(addr));                      break; // addr: $aa
        case kLDA3: lda_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kLDA4: lda_(absolute_addr(addr));                       break; // addr: $aaaa
        case kLDA5: lda_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kLDA6: lda_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kLDA7: lda_(indexed_indirect_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kLDA8: lda_(indirect_indexed_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kLDX1: ldx_(immediate_addr(addr)); break; // addr: #aa
        case kLDX2: ldx_(page_zero_addr(addr)); break; // addr: $aa
        case kLDX3: ldx_(indexed_pz_addr(addr, register_y_)); break; // addr: $aa,Y
        case kLDX4: ldx_(absolute_addr(addr)); break; // addr: $aaaa
        case kLDX5: ldx_(indexed_abs_addr(addr, register_y_)); break; // addr: $aaaa,Y

        case kLDY1: ldy_(immediate_addr(addr)); break; // addr: #aa
        case kLDY2: ldy_(page_zero_addr(addr)); break; // addr: $aa
        case kLDY3: ldy_(indexed_pz_addr(addr, register_x_)); break; // addr: $aa,X
        case kLDY4: ldy_(absolute_addr(addr)); break; // addr: $aaaa
        case kLDY5: ldy_(indexed_abs_addr(addr, register_x_)); break; // addr: $aaaa,X

        case kLSR1: lsr_();                                     break; // addr: A
        case kLSR2: lsr_(page_zero_addr(addr));                 break; // addr: $aa
        case kLSR3: lsr_(indexed_pz_addr(addr, register_x_));   break; // addr: $aa,X
        case kLSR4: lsr_(absolute_addr(addr));                  break; // addr: $aaaa
        case kLSR5: lsr_(indexed_abs_addr(addr, register_x_));  break; // addr: $aaaa,X

        case kNOP: nop_();  break;

        case kORA1: ora_(immediate_addr(addr));                     break; // addr: #aa
        case kORA2: ora_(page_zero_addr(addr));                     break; // addr: $aa
        case kORA3: ora_(indexed_pz_addr(addr, register_x_));       break; // addr: $aa,X
        case kORA4: ora_(absolute_addr(addr));                      break; // addr: $aaaa
        case kORA5: ora_(indexed_abs_addr(addr, register_x_));      break; // addr: $aaaa,X
        case kORA6: ora_(indexed_abs_addr(addr, register_y_));      break; // addr: $aaaa,Y
        case kORA7: ora_(indexed_indirect_addr(addr, register_x_)); break; // addr: ($aa,X)
        case kORA8: ora_(indirect_indexed_addr(addr, register_y_)); break; // addr: ($aa),Y

        case kPHA: pha_();  break;

        case kPHP: php_();  break;

        case kPLA: pla_();  break;

        case kPLP: plp_();  break;

        case kROL1: rol_(accumulator_);                         break; // addr: A
        case kROL2: rol_(page_zero_addr(addr));                 break; // addr: $aa
        case kROL3: rol_(indexed_pz_addr(addr, register_x_));   break; // addr: $aa,X
        case kROL4: rol_(absolute_addr(addr));                  break; // addr: $aaaa
        case kROL5: rol_(indexed_abs_addr(addr, register_x_));  break; // addr: $aaaa,X

        case kROR1: ror_(accumulator_);                         break; // addr: A
        case kROR2: ror_(page_zero_addr(addr));                 break; // addr: $aa
        case kROR3: ror_(indexed_pz_addr(addr, register_x_));   break; // addr: $aa,X
        case kROR4: ror_(absolute_addr(addr));                  break; // addr: $aaaa
        case kROR5: ror_(indexed_abs_addr(addr, register_x_));  break; // addr: $aaaa,X

        case kRTI: rti_();  break;

        case kRTS: rts_();  break;

        case kSAX1: sax_(indexed_indirect_addr(addr, register_x_)); break; // addr: ($aa,X)
        case kSAX2: sax_(page_zero_addr(addr));                     break; // addr: $aa
        case kSAX3: sax_(absolute_addr(addr));                      break; // addr: $aaaa
        case kSAX4: sax_(indexed_pz_addr(addr, register_y_));       break; // addr: $aa,Y

        case kSBC1: sbc_(immediate_addr(addr));                     break; // addr: #aa
        case kSBC2: sbc_(page_zero_addr(addr));                     break; // addr: $aa
        case kSBC3: sbc_(indexed_pz_addr(addr, register_x_));       break; // addr: $aa,X
        case kSBC4: sbc_(absolute_addr(addr));                      break; // addr: $aaaa
        case kSBC5: sbc_(indexed_abs_addr(addr, register_x_));      break; // addr: $aaaa,X
        case kSBC6: sbc_(indexed_abs_addr(addr, register_y_));      break; // addr: $aaaa,Y
        case kSBC7: sbc_(indexed_indirect_addr(addr, register_x_)); break; // addr: ($aa,X)
        case kSBC8: sbc_(indirect_indexed_addr(addr, register_y_)); break; // addr: ($aa),Y

        case kSEC: sec_();  break;

        case kSED: sed_();  break;

        case kSEI: sei_();  break;

        case kSTA1: sta_(page_zero_addr(addr));                     break; // addr: $aa
        case kSTA2: sta_(indexed_pz_addr(addr, register_x_));       break; // addr: $aa,X
        case kSTA4: sta_(absolute_addr(addr));                      break; // addr: $aaaa
        case kSTA5: sta_(indexed_abs_addr(addr, register_x_));      break; // addr: $aaaa,X
        case kSTA6: sta_(indexed_abs_addr(addr, register_y_));      break; // addr: $aaaa,Y
        case kSTA7: sta_(indexed_indirect_addr(addr, register_x_)); break; // addr: ($aa,X)
        case kSTA8: sta_(indirect_indexed_addr(addr, register_y_)); break; // addr: ($aa),Y

        case kSTX1: stx_(page_zero_addr(addr));                 break; // addr: $aa
        case kSTX2: stx_(indexed_pz_addr(addr, register_y_));    break; // addr: $aa,Y
        case kSTX3: stx_(absolute_addr(addr));                  break; // addr: $aaaa

        case kSTY1: sty_(page_zero_addr(addr));                 break; // addr: $aa
        case kSTY2: sty_(indexed_pz_addr(addr, register_x_));    break; // addr: $aa,X
        case kSTY3: sty_(absolute_addr(addr));                  break; // addr: $aaaa

        case kTAX: tax_();  break;

        case kTAY: tay_();  break;

        case kTSX: tsx_();  break;

        case kTXA: txa_();  break;

        case kTXS: txs_();  break;

        case kTYA: tya_();  break;

        default: nop_(); break;
    }
}

inline void CPU::store_(address_t addr, byte_t operand)
{
    memory_[addr] = operand;
}

inline void CPU::store_(address_t addr, address_t addr_value)
{
    memory_[addr + 1] = static_cast<byte_t>(addr_value >> 8);
    memory_[addr] = static_cast<byte_t>(0xFF & addr_value);
}

inline byte_t CPU::load_(address_t addr)
{
    return memory_[addr];
}

inline address_t CPU::load_addr_(address_t addr)
{
    return static_cast<address_t>(memory_[addr + 1]) << 8 | (0xFF & static_cast<address_t>(memory_[addr]));
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

inline void CPU::load_stack_(byte_t & dest)
{
    ++stack_pointer_;
    dest = load_(0x0100 | stack_pointer_);
}

inline void CPU::load_stack_(address_t & dest)
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
    return static_cast<address_t>(memory_[addr_h]) << 8 | memory_[addr];
}

// $00
inline address_t CPU::page_zero_addr(address_t addr)
{
    return 0xFF & addr;
}

inline address_t CPU::indirect_pz_addr(byte_t addr)
{
    return static_cast<address_t>(memory_[static_cast<byte_t>(addr + 1)]) << 8 | memory_[addr];
}

// $(00)
inline address_t CPU::indexed_pz_addr(address_t addr, byte_t index)
{
    return  static_cast<byte_t>(static_cast<byte_t>(0xFF & addr) + index);
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
    std::ostringstream oss;

    byte_t addr_l = static_cast<byte_t>(0xFF & addr);

    switch(type)
    {
    case ops::kImmediate:
        oss << "#$" << _str(addr_l);
        break;

    case ops::kZeroPage:
        oss << "$" << _str(addr_l) << " = " << _str(load_(addr));
        break;

    case ops::kZeroPageX:
        addr = indexed_pz_addr(addr, register_x_);
        oss << "$" << _str(addr_l) << ",X @ ";
        addr_l = static_cast<byte_t>(0xFF & addr);
        oss << _str(addr_l) << " = " << _str(load_(addr));
        break;

    case ops::kAbsolute:
        oss << "$" << _str(addr);
        if (addr < 0x8000)
        {
            oss << " = " << _str(load_(addr));
        }
        break;

    case ops::kIndirect:
        oss << "($" << _str(addr) << ")"
            << " = " << _str(indirect_addr(addr));
        break;

    case ops::kIndirectX:
        addr = indexed_indirect_addr(addr, register_x_);
        oss << "($" << _str(addr_l)
            << ",X) @ " << _str(addr)
            << " = " << _str(load_(addr));
        break;

    default:
        break;
    }

    return oss.str();
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

void CPU::rol_(byte_t & operand)
{
    byte_t carry = operand & kNegative;
    operand <<= 1;
    if (get_status_(kCarry)) operand |= kCarry;
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

void CPU::ror_(byte_t & operand)
{
    byte_t carry = operand & kCarry;
    operand >>= 1;
    if (get_status_(kCarry)) operand |= kNegative;
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
