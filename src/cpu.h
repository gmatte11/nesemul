#pragma once

#include "types.h"

#include <array>
#include <vector>
#include <tuple>
#include <string>

class BUS;

struct CPU_State
{
    // status flags (NOxBDIZC)
    enum StatusFlags : byte_t
    {
        kCarry = 1 << 0,
        kZero = 1 << 1,
        kIntDisable = 1 << 2,
        kDecimal = 1 << 3,
        kBreak = 1 << 4,
        kOverflow = 1 << 6,
        kNegative = 1 << 7,
    };

    unsigned long long cycle_ = 0ull;
    int idle_ticks_ = 0;

    address_t program_counter_ = 0x0000;
    address_t old_pc_ = 0x0000;

    // registers
    byte_t accumulator_ = 0x0;
    byte_t register_x_ = 0x0;
    byte_t register_y_ = 0x0;
    byte_t status_ = 0x24;
    byte_t stack_pointer_ = 0xFD;

    struct instruction
    {
        byte_t opcode = 0xFF;
        byte_t op1;
        byte_t op2;

        address_t to_addr() const { return static_cast<address_t>(op2) << 8 | op1; }
    } instr_;
};

// Emulate 6502 CPU
class CPU : private CPU_State
{
public:
    CPU() = default;

    void init(BUS* bus) { bus_ = bus; }

    // Execute next instruction from the program
    void step();
    void reset();

    void interrupt(bool nmi = false);

    CPU_State const& get_state() const { return *this; }

    std::array<std::array<char, 80>, 64> log_ring_;
    int log_idx_ = 0;

private:
    void exec_(byte_t opcode, address_t addr);
    void irq_();
    void nmi_();
    void log_(byte_t opcode, address_t addr);

    int idle_ticks_from_branching_(byte_t opcode, address_t addr);
    int idle_ticks_from_addressing_(byte_t addr_mode, address_t addr);

    //interrupt
    std::pair<bool, bool> int_ = {false, false};

    // bus
    BUS* bus_ = nullptr;

    // bus access
    void store_(address_t addr, byte_t operand);
    void store_(address_t addr, address_t addr_value);
    byte_t load_(address_t addr);
    address_t load_addr_(address_t addr);

    void store_stack_(byte_t operand);
    void store_stack_(address_t addr);
    void load_stack_(byte_t& dest);
    void load_stack_(address_t& dest);

    void set_status_(byte_t flag_mask, bool value);
    bool get_status_(byte_t flag_mask);

    // addressing
    byte_t immediate_addr(address_t addr); // #$aa
    address_t absolute_addr(address_t addr); // $aaaa
    address_t indexed_abs_addr(address_t addr, byte_t index); // $aaaa,X
    address_t indirect_addr(address_t addr); // ($aaaa)
    address_t page_zero_addr(address_t addr); // $aa
    address_t indirect_pz_addr(byte_t addr); // ($aa)
    address_t indexed_pz_addr(address_t addr, byte_t index); // $aa,X
    address_t indexed_indirect_addr(address_t addr, byte_t index); // ($aa,X)
    address_t indirect_indexed_addr(address_t addr, byte_t index); // ($aa),Y
    std::string debug_addr_(byte_t type, address_t addr);

    bool is_branch_(byte_t opcode) const;

    // operations
    void adc_(byte_t operand);
    void and_(byte_t operand);
    void asl_(byte_t& operand);
    void bcc_(address_t addr);
    void bcs_(address_t addr);
    void beq_(address_t addr);
    void bit_(address_t addr);
    void bmi_(address_t addr);
    void bne_(address_t addr);
    void bpl_(address_t addr);
    void brk_();
    void bvc_(address_t addr);
    void bvs_(address_t addr);
    void clc_();
    void cld_();
    void cli_();
    void clv_();
    void cmp_(byte_t operand);
    void cpx_(byte_t operand);
    void cpy_(byte_t operand);
    void dcp_(address_t addr);
    void dec_(address_t addr);
    void dex_();
    void dey_();
    void eor_(byte_t operand);
    void inc_(address_t addr);
    void inx_();
    void iny_();
    void isb_(address_t addr);
    void jmp_(address_t addr);
    void jsr_(address_t addr);
    void lax_(address_t addr);
    void lda_(byte_t operand);
    void ldx_(byte_t operand);
    void ldy_(byte_t operand);
    void lsr_(byte_t& operand);
    void nop_();
    void ora_(byte_t operand);
    void pha_();
    void php_();
    void pla_();
    void plp_();
    void rla_(address_t addr);
    void rol_(byte_t& operand);
    void ror_(byte_t& operand);
    void rra_(address_t addr);
    void rti_();
    void rts_();
    void sax_(address_t addr);
    void sbc_(byte_t operand);
    void sbc_(address_t addr);
    void sec_();
    void sed_();
    void sei_();
    void slo_(address_t addr);
    void sre_(address_t addr);
    void sta_(address_t addr);
    void stx_(address_t addr);
    void sty_(address_t addr);
    void tax_();
    void tay_();
    void tsx_();
    void txa_();
    void txs_();
    void tya_();
};
