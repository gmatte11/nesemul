#ifndef __NESEMUL_CPU_H__
#define __NESEMUL_CPU_H__

#include <types.h>

#include <array>

// Emulate 6502 CPU
class CPU
{
public:
    // Execute next instruction from the program
    void next();

    inline byte_t *data() { return memory_.data(); }

private:
    void exec_(byte_t opcode, address_t addr);

    // registers
    byte_t accumulator_;
    byte_t register_x_;
    byte_t register_y_;
    byte_t status_;
    address_t program_counter_;
    address_t stack_pointer_;

    // memory buffer
    std::array<byte_t, 0x10000> memory_;

    // memory access
    void store_(address_t addr, byte_t operand);
    byte_t load_(address_t addr);

    // status flags
    enum StatusFlags : byte_t
    {
        kCarry      = 1 << 0,
        kZero       = 1 << 1,
        kIntDisable = 1 << 2,
        kDecimal    = 1 << 3,
        kBreak      = 1 << 4,
        kOverflow   = 1 << 6,
        kNegative   = 1 << 7,
    };

    void set_status_(byte_t flag_mask, bool value);
    bool get_status_(byte_t flag_mask);

    // operations
    void adc_(address_t addr);
    void and_(address_t addr);
    void asl_();
    void asl_(address_t addr);
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
    void cmp_(address_t addr);
    void cpx_(address_t addr);
    void cpy_(address_t addr);
    void dec_(address_t addr);
    void dex_();
    void dey_();
    void eor_(address_t addr);
    void inc_(address_t addr);
    void inx_();
    void iny_();
    void jmp_(address_t addr);
    void jsr_(address_t addr);
    void lda_(address_t addr);
    void ldx_(address_t addr);
    void ldy_(address_t addr);
    void lsr_();
    void lsr_(address_t addr);
    void nop_();
    void ora_(address_t addr);
    void pha_();
    void php_();
    void pla_();
    void plp_();
    void rol_();
    void rol_(address_t addr);
    void ror_();
    void ror_(address_t addr);
    void rti_();
    void rts_();
    void sbc_(address_t addr);
    void sec_();
    void sed_();
    void sei_();
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

#endif // __NESEMUL_CPU_H__
