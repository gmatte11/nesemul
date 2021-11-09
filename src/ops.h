#ifndef __NESEMUL_OPS_H__
#define __NESEMUL_OPS_H__

#include "types.h"

#include <string>

namespace ops
{
struct metadata
{
    byte_t operation;
    byte_t addressing;
    byte_t timing;
    const char* str;

    byte_t get_size() const;
};

const metadata& opcode_data(byte_t opcode);

enum Addressing : byte_t
{
    kNone = 0x00,      // a.k.a Implied or Accumulator
    kImmediate = 0x01, // #$00 
    kZeroPage = 0x02,  // $00  
    kZeroPageX = 0x03, // $00,X
    kZeroPageY = 0x0A, // $00,Y
    kAbsolute = 0x04,  // $0000
    kAbsoluteX = 0x05, // $0000,X
    kAbsoluteY = 0x06, // $0000,Y
    kIndirect = 0x07,  // ($0000)
    kIndirectX = 0x08, // ($00,X)
    kIndirectY = 0x09, // ($00),Y
    kRelative = 0x0B   // $0000 (Branches specific)
};

enum Operations : byte_t
{
    kUKN, // unknown
    kADC, // add with carry
    kAND, // logical and
    kASL, // arithmetic shift left
    kBCC, // branch if carry clear
    kBCS, // branch if carry set
    kBEQ, // branch if equal
    kBIT, // bit test
    kBMI, // branch if minus
    kBNE, // branch if not equal
    kBPL, // branch if positive
    kBRK, // break
    kBVC, // branch if overflow clear
    kBVS, // branch if overflow set
    kCLC, // clear carry flag
    kCLD, // clear decimal mode
    kCLI, // clear interrupt disable
    kCLV, // clear overflow flag
    kCMP, // compare
    kCPX, // compare X register
    kCPY, // compare Y register
    kDEC, // decrement memory
    kDEX, // decrement X register
    kDEY, // decrement Y register
    kEOR, // exclusive or
    kINC, // increment memory
    kINX, // increment X register
    kINY, // increment Y register
    kJMP, // jump
    kJSR, // jump to subroutine
    kLDA, // load accumulator
    kLDX, // load X register
    kLDY, // load Y register
    kLSR, // logical shift right
    kNOP, // No-op
    kORA, // logical inclusive or
    kPHA, // push accumulator
    kPHP, // push processor status
    kPLA, // pull accumulator
    kPLP, // pull processor status
    kROL, // rotate left
    kROR, // rotate right
    kRTI, // return from interrupt
    kRTS, // return from subroutine
    kSBC, // substract with carry
    kSEC, // set carry flag
    kSED, // set decimal flag
    kSEI, // set interrupt disable
    kSTA, // store accumulator
    kSTX, // store X register
    kSTY, // store Y register
    kTAX, // transfer accumulator to X register
    kTAY, // transfer accumulator to Y register
    kTSX, // transfer stack pointer to X register
    kTXA, // transfer X register to accumulator
    kTXS, // transfer X to stack pointer
    kTYA, // transfer Y register to accumulator

    // illegal opcodes
    kDCP, // decrement then compare
    kISB, // increment then substract
    kLAX, // load accumulator and transfer to register X 
    kRLA, // rotate left then logical and
    kRRA, // rotate right then add
    kSAX, // store bitwise and of accumulator and register X 
    kSLO, // shift left then logical or
    kSRE, // logical shift right then exclusive or
};




}

#endif // __NESEMUL_OPS_H__
