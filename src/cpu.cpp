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
    inline byte_t immediate_addr(address_t addr)
    {
        return 0xFF & addr;
    }

    inline address_t absolute_addr(address_t addr)
    {
        return addr;
    }

    inline address_t indexed_abs_addr(address_t addr, byte_t index)
    {
        return addr + index;
    }

    inline address_t indirect_addr(address_t addr)
    {
        //return static_cast<address_t>(memory_[addr + 1]) << 8 | memory_[addr];
        return addr;
    }

    inline address_t page_zero_addr(address_t addr)
    {
        return 0xFF & addr;
    }

    inline address_t indexed_pz_addr(address_t addr, byte_t index)
    {
        return 0xFF & (addr + index);
    }

    inline address_t indexed_indirect_addr(address_t addr, byte_t index)
    {
        return indirect_addr(indexed_pz_addr(addr, index));
    }

    inline address_t indirect_indexed_addr(address_t addr, byte_t index)
    {
        return indirect_addr(page_zero_addr(addr)) + index;
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
        << "  " << std::setw(2) << static_cast<unsigned int>(data.opcode);

    if (opcode_data(data.opcode).size > 1)
        std::cout << "  " << std::setw(2) << static_cast<unsigned int>(data.addr_l);
    else
        std::cout << "    ";

    if (opcode_data(data.opcode).size > 2)
        std::cout << "  " << std::setw(2) << static_cast<unsigned int>(data.addr_h);
    else
        std::cout << "    ";

    std::cout
        << "  " << opcode_data(data.opcode).str << " $    ";

    exec_(data.opcode, addr);

    std::cout
        << "\tA:" << std::setw(2) << static_cast<unsigned int>(accumulator_)
        << " X:" << std::setw(2) << static_cast<unsigned int>(register_x_)
        << " Y:" << std::setw(2) << static_cast<unsigned int>(register_y_)
        << " P:" << std::setw(2) << static_cast<unsigned int>(status_)
        << " SP:" << std::setw(2) << static_cast<unsigned int>(stack_pointer_)
        << '\n';

    if (program_counter_ < 0x8000)
    {
        std::ostringstream oss;
        oss << std::hex << "Invalid jump to " << program_counter_;
        throw std::runtime_error(oss.str());
    }

    program_counter_ += opcode_data(data.opcode).size;
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
        case kADC7: adc_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kADC8: adc_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kAND1: and_(immediate_addr(addr));                     break; // addr: #aa
        case kAND2: and_(page_zero_addr(addr));                     break; // addr: $aa
        case kAND3: and_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kAND4: and_(absolute_addr(addr));                      break; // addr: $aaaa
        case kAND5: and_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kAND6: and_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kAND7: and_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kAND8: and_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

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
        case kCMP7: cmp_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kCMP8: cmp_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

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
        case kEOR7: eor_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kEOR8: eor_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kINC1: inc_(page_zero_addr(addr));                 break; // addr: $aa
        case kINC2: inc_(indexed_pz_addr(addr, register_x_));    break; // addr: $aa,X
        case kINC3: inc_(absolute_addr(addr));                  break; // addr: $aaaa
        case kINC4: inc_(indexed_abs_addr(addr, register_x_));   break; // addr: $aaaa,X

        case kINX: inx_();  break;

        case kINY: iny_();  break;

        case kJMP1: jmp_(absolute_addr(addr)); break; // addr: $aaaa
        case kJMP2: jmp_(indirect_addr(addr)); break; // addr: ($aaaa)

        case kJSR: jsr_(absolute_addr(addr));   break; // addr: $aaaa

        case kLDA1: lda_(immediate_addr(addr));                      break; // addr: #aa
        case kLDA2: lda_(page_zero_addr(addr));                      break; // addr: $aa
        case kLDA3: lda_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kLDA4: lda_(absolute_addr(addr));                       break; // addr: $aaaa
        case kLDA5: lda_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kLDA6: lda_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kLDA7: lda_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kLDA8: lda_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

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
        case kORA7: ora_(indirect_indexed_addr(addr, register_x_)); break; // addr: ($aa,X)
        case kORA8: ora_(indexed_indirect_addr(addr, register_y_)); break; // addr: ($aa),Y

        case kPHA: pha_();  break;

        case kPHP: php_();  break;

        case kPLA: pla_();  break;

        case kPLP: plp_();  break;

        case kROL1: rol_();                                     break; // addr: A
        case kROL2: rol_(page_zero_addr(addr));                 break; // addr: $aa
        case kROL3: rol_(indexed_pz_addr(addr, register_x_));    break; // addr: $aa,X
        case kROL4: rol_(absolute_addr(addr));                  break; // addr: $aaaa
        case kROL5: rol_(indexed_abs_addr(addr, register_x_));   break; // addr: $aaaa,X

        case kROR1: ror_();                                     break; // addr: A
        case kROR2: ror_(page_zero_addr(addr));                 break; // addr: $aa
        case kROR3: ror_(indexed_pz_addr(addr, register_x_));    break; // addr: $aa,X
        case kROR4: ror_(absolute_addr(addr));                  break; // addr: $aaaa
        case kROR5: ror_(indexed_abs_addr(addr, register_x_));   break; // addr: $aaaa,X

        case kRTI: rti_();  break;

        case kRTS: rts_();  break;

        case kSBC1: sbc_(immediate_addr(addr));                     break; // addr: #aa
        case kSBC2: sbc_(page_zero_addr(addr));                     break; // addr: $aa
        case kSBC3: sbc_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kSBC4: sbc_(absolute_addr(addr));                      break; // addr: $aaaa
        case kSBC5: sbc_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kSBC6: sbc_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kSBC7: sbc_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kSBC8: sbc_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

        case kSEC: sec_();  break;

        case kSED: sed_();  break;

        case kSEI: sei_();  break;

        case kSTA1: sta_(page_zero_addr(addr));                     break; // addr: $aa
        case kSTA2: sta_(indexed_pz_addr(addr, register_x_));        break; // addr: $aa,X
        case kSTA4: sta_(absolute_addr(addr));                      break; // addr: $aaaa
        case kSTA5: sta_(indexed_abs_addr(addr, register_x_));       break; // addr: $aaaa,X
        case kSTA6: sta_(indexed_abs_addr(addr, register_y_));       break; // addr: $aaaa,Y
        case kSTA7: sta_(indirect_indexed_addr(addr, register_x_));  break; // addr: ($aa,X)
        case kSTA8: sta_(indexed_indirect_addr(addr, register_y_));  break; // addr: ($aa),Y

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

        default:
            std::ostringstream oss;
                oss << std::hex << "Execution alted on unsupported opcode: " << static_cast<uint16_t>(opcode);
            throw std::runtime_error(oss.str());
            break;
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
    --stack_pointer_;
    store_(0x0100 + stack_pointer_, operand);
}

inline void CPU::store_stack_(address_t addr)
{
    stack_pointer_ -= 2;
    store_(0x0100 + stack_pointer_, addr);
}

inline void CPU::load_stack_(byte_t & dest)
{
    dest = load_(0x0100 + stack_pointer_);
    ++stack_pointer_;
}

inline void CPU::load_stack_(address_t & dest)
{
    dest = load_addr_(0x0100 + stack_pointer_);
    stack_pointer_ += 2;
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

void CPU::adc_(byte_t operand)
{
    unsigned short result = static_cast<unsigned short>(accumulator_) + operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, result & kNegative);
    set_status_(kCarry, result & (1 << 8));
    set_status_(kOverflow, result & (1 << 9));
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
    byte_t operand = accumulator_ & load_(addr);
    set_status_(kZero, operand == 0);
    set_status_(kNegative, operand & kNegative);
    set_status_(kOverflow, operand & kOverflow);
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
    if (accumulator_ <  operand) { set_status_(kNegative, true); set_status_(kZero | kCarry, false); }
    if (accumulator_ == operand) { set_status_(kNegative, false); set_status_(kZero | kCarry, true); }
    if (accumulator_ >  operand) { set_status_(kNegative | kZero, false); set_status_(kCarry, true); }
}

void CPU::cmp_(address_t addr)
{
    cmp_(load_(addr));
}

void CPU::cpx_(byte_t operand)
{
    if (register_x_ <  operand) { set_status_(kNegative, true); set_status_(kZero | kCarry, false); }
    if (register_x_ == operand) { set_status_(kNegative, false); set_status_(kZero | kCarry, true); }
    if (register_x_ >  operand) { set_status_(kNegative | kZero, false); set_status_(kCarry, true); }
}

void CPU::cpx_(address_t addr)
{
    cpx_(load_(addr));
}

void CPU::cpy_(byte_t operand)
{
    if (register_y_ <  operand) { set_status_(kNegative, true); set_status_(kZero | kCarry, false); }
    if (register_y_ == operand) { set_status_(kNegative, false); set_status_(kZero | kCarry, true); }
    if (register_y_ >  operand) { set_status_(kNegative | kZero, false); set_status_(kCarry, true); }
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
    store_stack_(program_counter_);
    program_counter_ = addr;
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
}

void CPU::plp_()
{
    load_stack_(status_);
}

void CPU::rol_()
{
    set_status_(kCarry, accumulator_ & kNegative);
    accumulator_ <<= 1;
    if (get_status_(kCarry)) accumulator_ &= kCarry;
}

void CPU::rol_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kNegative);
    operand <<= 1;
    if (get_status_(kCarry)) operand &= kCarry;
    store_(addr, operand);
}

void CPU::ror_()
{
    set_status_(kCarry, accumulator_ & kCarry);
    accumulator_ >>= 1;
    if (get_status_(kCarry)) accumulator_ &= kNegative;
}

void CPU::ror_(address_t addr)
{
    byte_t operand = load_(addr);
    set_status_(kCarry, operand & kCarry);
    operand >>= 1;
    if (get_status_(kCarry)) operand &= kNegative;
    store_(addr, operand);
}

void CPU::rti_()
{
}

void CPU::rts_()
{
    load_stack_(program_counter_);
}

void CPU::sbc_(byte_t operand)
{
    unsigned short result = static_cast<unsigned short>(accumulator_) - operand;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, result & kNegative);
    set_status_(kCarry, result & (1 << 8));
    set_status_(kOverflow, result & (1 << 9));
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
    set_status_(kZero, stack_pointer_ == 0);
    set_status_(kNegative, stack_pointer_ & kNegative);
}

void CPU::tya_()
{
    accumulator_ = register_y_;
    set_status_(kZero, accumulator_ == 0);
    set_status_(kNegative, accumulator_ & kNegative);
}
