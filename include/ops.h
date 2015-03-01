#ifndef __NESEMUL_OPS_H__
#define __NESEMUL_OPS_H__

#include <types.h>

namespace ops
{
    struct
    {
        CPU::byte_t size;
        CPU::byte_t timing;
    } metadata;

    metadata opcode_data(byte_t opcode);

    enum Codes : byte_t
    {
    //ADC - add with carry
        kADC1 = 0x69, // ops: 2, addr: #aa
        kADC2 = 0x65, // ops: 2, addr: $aa
        kADC3 = 0x75, // ops: 2, addr: $aa,X
        kADC4 = 0x60, // ops: 3, addr: $aaaa
        kADC5 = 0x70, // ops: 3, addr: $aaaa,X
        kADC6 = 0x79, // ops: 3, addr: $aaaa,Y
        kADC7 = 0x61, // ops: 2, addr: ($aa,X)
        kADC8 = 0x71, // ops: 2, addr: ($aa),Y

    //AND - logical and
        kAND1 = 0x29, // ops: 2, addr: #aa
        kAND2 = 0x25, // ops: 2, addr: $aa
        kAND3 = 0x35, // ops: 2, addr: $aa,X
        kAND4 = 0x2D, // ops: 3, addr: $aaaa
        kAND5 = 0x3D, // ops: 3, addr: $aaaa,X
        kAND6 = 0x39, // ops: 3, addr: $aaaa,Y
        kAND7 = 0x21, // ops: 2, addr: ($aa,X)
        kAND8 = 0x31, // ops: 2, addr: ($aa),Y

    //ASL - arithmetic shift left
        kASL1 = 0x0A, // ops: 1, addr: A
        kASL2 = 0x06, // ops: 2, addr: $aa
        kASL3 = 0x16, // ops: 2, addr: $aa,X
        kASL4 = 0x0E, // ops: 3, addr: $aaaa
        kASL5 = 0x1E, // ops: 3, addr: $aaaa,X

    //BCC - branch if carry clear
        kBCC  = 0x90, // ops: 2, addr: $aa

    //BCS - branch if carry set
        kBCS  = 0xB0, // ops: 2, addr: $aa

    //BEQ - branch if equal
        kBEQ  = 0xF0, // ops: 2, addr: $aa

    //BIT - bit test
        kBIT  = 0x24, // ops: 2, addr: $aa
        kBIT  = 0x2C, // ops: 3, addr: $aaaa

    //BMI - branch if minus
        kBMI  = 0x30, // ops: 2, addr: $aa

    //BNE - branch if not equal
        kBNE  = 0xD0, // ops: 2, addr: $aa

    //BPL - branch on plus
        kBPL  = 0x10, // ops: 2, addr: $aa

    //BRK - break
        kBRK  = 0x00, // ops: 1

    //BVC - branch if overflow clear
        kBVC  = 0x50, // ops: 2, addr: $aa

    //BVS - branch if overflow set
        kBVS  = 0x70, // ops: 2, addr: $aa

    //CLC - clear carry
        kCLC  = 0x18, // ops: 1

    //CLD - clear decimal mode
        kCLD  = 0xD8, // ops: 1

    //CLI - clear interrupt disable
        kCLI  = 0x58, // ops: 1

    //CLV - clear overflow flag
        kCLV  = 0xB8, // ops: 1

    //CMP - compare (memory and accumulator)
        kCMP1 = 0xC9, // ops: 2, addr: #aa
        kCMP2 = 0xC5, // ops: 2, addr: $aa
        kCMP3 = 0xD5, // ops: 2, addr: $aa,X
        kCMP4 = 0xCD, // ops: 3, addr: $aaaa
        kCMP5 = 0xDD, // ops: 3, addr: $aaaa,X
        kCMP6 = 0xD9, // ops: 3, addr: $aaaa,Y
        kCMP7 = 0xC1, // ops: 2, addr: ($aa,X)
        kCMP8 = 0xD1, // ops: 2, addr: ($aa),Y

    //CPX - compare (memory and register X)
        kCPX1 = 0xE0, // ops: 2, addr: #aa
        kCPX2 = 0xE4, // ops: 2, addr: $aa
        kCPX3 = 0xEC, // ops: 3, addr: $aaaa

    //CPY - compare (memory and register Y)
        kCPY1 = 0xC0, // ops: 2, addr: #aa
        kCPY2 = 0xC4, // ops: 2, addr: $aa
        kCPY3 = 0xCC, // ops: 3, addr: $aaaa

    //DEC - decrement source
        kDEC1 = 0xC6, // ops: 2, addr: $aa
        kDEC2 = 0xD6, // ops: 2, addr: $aa,X
        kDEC3 = 0xCE, // ops: 3, addr: $aaaa
        kDEC4 = 0xDE, // ops: 3, addr: $aaaa,X

    //DEX - decrement register X
        kDEX  = 0xCA, // ops: 1

    //DEY - decrement register Y
        kDEY  = 0x88, // ops: 1

    //EOR - logical xor
        kEOR1 = 0x49, // ops: 2, addr: #aa
        kEOR2 = 0x45, // ops: 2, addr: $aa
        kEOR3 = 0x55, // ops: 2, addr: $aa,X
        kEOR4 = 0x4D, // ops: 3, addr: $aaaa
        kEOR5 = 0x5D, // ops: 3, addr: $aaaa,X
        kEOR6 = 0x59, // ops: 3, addr: $aaaa,Y
        kEOR7 = 0x41, // ops: 2, addr: ($aa,X)
        kEOR8 = 0x51, // ops: 2, addr: ($aa),Y

    //INC - increment memory
        kINC1 = 0xE6, // ops: 2, addr: $aa
        kINC2 = 0xF6, // ops: 2, addr: $aa,X
        kINC3 = 0xEE, // ops: 3, addr: $aaaa
        kINC4 = 0xFE, // ops: 3, addr: $aaaa,X

    //INX - increment register X
        kINX  = 0xE8, // ops: 1

    //INY - increment register Y
        kINY  = 0xC8, // ops: 1

    //JMP - jump
        kJMP1 = 0x4C, // ops: 3, addr: $aaaa
        kJMP2 = 0x6C, // ops: 3, addr: ($aaaa)

    //JSR - jump to subroutine
        kJSR  = 0x20, // ops: 3, addr: $aaaa

    //LDA - load accumulator
        kLDA1 = 0xA9, // ops: 2, addr: #aa
        kLDA2 = 0xA5, // ops: 2, addr: $aa
        kLDA3 = 0xB5, // ops: 2, addr: $aa,X
        kLDA4 = 0xAD, // ops: 3, addr: $aaaa
        kLDA5 = 0xBD, // ops: 3, addr: $aaaa,X
        kLDA6 = 0xB9, // ops: 3, addr: $aaaa,Y
        kLDA7 = 0xA1, // ops: 2, addr: ($aa,X)
        kLDA8 = 0xB1, // ops: 2, addr: ($aa),Y

    //LDX - load register X
        kLDX1 = 0xA2, // ops: 2, addr: #aa
        kLDX2 = 0xA6, // ops: 2, addr: $aa
        kLDX3 = 0xB6, // ops: 2, addr: $aa,Y
        kLDX4 = 0xAE, // ops: 3, addr: $aaaa
        kLDX5 = 0xBE, // ops: 3, addr: $aaaa,Y

    //LDY - load register Y
        kLDY1 = 0xA0, // ops: 2, addr: #aa
        kLDY2 = 0xA4, // ops: 2, addr: $aa
        kLDY3 = 0xB4, // ops: 2, addr: $aa,X
        kLDY4 = 0xAC, // ops: 3, addr: $aaaa
        kLDY5 = 0xBC, // ops: 3, addr: $aaaa,X

    //LSR - logical shift right
        kLSR1 = 0x4A, // ops: 1, addr: A
        kLSR2 = 0x46, // ops: 2, addr: $aa
        kLSR3 = 0x56, // ops: 2, addr: $aa,X
        kLSR4 = 0x4E, // ops: 3, addr: $aaaa
        kLSR5 = 0x5E, // ops: 3, addr: $aaaa,X

    //NOP - no-op
        kNOP  = 0xEA, // ops: 1

    //ORA - logical or
        kORA1 = 0x09, // ops: 2, addr: #aa
        kORA2 = 0x05, // ops: 2, addr: $aa
        kORA3 = 0x15, // ops: 2, addr: $aa,X
        kORA4 = 0x0D, // ops: 3, addr: $aaaa
        kORA5 = 0x1D, // ops: 3, addr: $aaaa,X
        kORA6 = 0x19, // ops: 3, addr: $aaaa,Y
        kORA7 = 0x01, // ops: 2, addr: ($aa,X)
        kORA8 = 0x11, // ops: 2, addr: ($aa),Y

    //PHA - push accumulator
        kPHA  = 0x48, // ops: 1

    //PHP - push processor status register
        kPHP  = 0x08, // ops: 1

    //PLA - pull accumulator
        kPLA  = 0x68, // ops: 1

    //PLP - pull processor status register
        kPLP  = 0x28, // ops: 1

    //ROL - rotate left
        kROL1 = 0x2A, // ops: 1, addr: A
        kROL2 = 0x26, // ops: 2, addr: $aa
        kROL3 = 0x36, // ops: 2, addr: $aa,X
        kROL4 = 0x2E, // ops: 3, addr: $aaaa
        kROL5 = 0x3E, // ops: 3, addr: $aaaa,X

    //ROR - rotate right
        kROR1 = 0x6A, // ops: 1, addr: A
        kROR2 = 0x66, // ops: 2, addr: $aa
        kROR3 = 0x76, // ops: 2, addr: $aa,X
        kROR4 = 0x6E, // ops: 3, addr: $aaaa
        kROR5 = 0x7E, // ops: 3, addr: $aaaa,X

    //RTI - return from interrupt
        kRTI  = 0x40, // ops: 1

    //RTS - return from subroutine
        kRTS  = 0x60, // ops: 1

    //SBC - substract with carry
        kSBC1 = 0xE9, // ops: 2, addr: #aa
        kSBC2 = 0xE5, // ops: 2, addr: $aa
        kSBC3 = 0xF5, // ops: 2, addr: $aa,X
        kSBC4 = 0xED, // ops: 3, addr: $aaaa
        kSBC5 = 0xFD, // ops: 3, addr: $aaaa,X
        kSBC6 = 0xF9, // ops: 3, addr: $aaaa,Y
        kSBC7 = 0xE1, // ops: 2, addr: ($aa,X)
        kSBC8 = 0xF1, // ops: 2, addr: ($aa),Y

    //SEC - set carry flag
        kSEC  = 0x38, // ops: 1

    //SED - set decimal mode
        kSED  = 0xF8, // ops: 1

    //SEI - set interrupt disable
        kSEI  = 0x78, // ops: 1

    //STA - store accumulator
        kSTA1 = 0x85, // ops: 2, addr: $aa
        kSTA2 = 0x95, // ops: 2, addr: $aa,X
        kSTA4 = 0x8D, // ops: 3, addr: $aaaa
        kSTA5 = 0x9D, // ops: 3, addr: $aaaa,X
        kSTA6 = 0x99, // ops: 3, addr: $aaaa,Y
        kSTA7 = 0x81, // ops: 2, addr: ($aa,X)
        kSTA8 = 0x91, // ops: 2, addr: ($aa),Y

    //STX - store register X
        kSTX1 = 0x86, // ops: 2, addr: $aa
        kSTX2 = 0x96, // ops: 2, addr: $aa,Y
        kSTX3 = 0x8E, // ops: 3, addr: $aaaa

    //STY - store register Y
        kSTY1 = 0x84, // ops: 2, addr: $aa
        kSTY2 = 0x94, // ops: 2, addr: $aa,X
        kSTY3 = 0x8C, // ops: 3, addr: $aaaa

    //TAX - transfer accumulator to register X
        kTAX  = 0xAA, // ops: 1

    //TAY - transfer accumulator to register Y
        kTAY  = 0xA8, // ops: 1

    //TSX - transfer stack to X
        kTSX  = 0xBA, // ops: 1

    //TXA - transfer register X to accumulator
        kTXA  = 0x8A, // ops: 1

    //TXS - transfer register X to stack
        kTXS  = 0x9A, // ops: 1

    //TYA - transfer register Y to accumulator
        kTYA  = 0x98  // ops: 1
    };
}

#endif // __NESEMUL_OPS_H__
