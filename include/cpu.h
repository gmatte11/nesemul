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
    byte_t store_(address_t addr, byte_t value);
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
    void asl_(address_t addr);
    void bcc_(address_t addr);
    void bcs_(address_t addr);
    void beq_(address_t addr);
    void bit_(address_t addr);
    void bmi_(address_t addr);
    void bne_(address_t addr);
    void bpl_(address_t addr);
    void brk_(address_t addr);
    void bvc_(address_t addr);
    void bvs_(address_t addr);
    void clc_(address_t addr);
    void cld_(address_t addr);
    void cli_(address_t addr);
    void clv_(address_t addr);
    void cmp_(address_t addr);
    void cpx_(address_t addr);
    void cpy_(address_t addr);
    void dec_(address_t addr);
    void dex_(address_t addr);
    void dey_(address_t addr);
    void eor_(address_t addr);
    void inc_(address_t addr);
    void inx_(address_t addr);
    void iny_(address_t addr);
    void jmp_(address_t addr);
    void jsr_(address_t addr);
    void lda_(address_t addr);
    void ldx_(address_t addr);
    void ldy_(address_t addr);
    void lsr_(address_t addr);
    void nop_(address_t addr);
    void ora_(address_t addr);
    void pha_(address_t addr);
    void php_(address_t addr);
    void pla_(address_t addr);
    void plp_(address_t addr);
    void rol_(address_t addr);
    void ror_(address_t addr);
    void rti_(address_t addr);
    void rts_(address_t addr);
    void sbc_(address_t addr);
    void sec_(address_t addr);
    void sei_(address_t addr);
    void sta_(address_t addr);
    void stx_(address_t addr);
    void sty_(address_t addr);
    void tax_(address_t addr);
    void tay_(address_t addr);
    void tsx_(address_t addr);
    void txa_(address_t addr);
    void txs_(address_t addr);
    void tya_(address_t addr);
};

#endif // __NESEMUL_CPU_H__
