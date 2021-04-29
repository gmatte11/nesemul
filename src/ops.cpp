#include "ops.h"

namespace ops
{

enum
{
    Non = ops::kNone,
    Imm = ops::kImmediate,
    ZPg = ops::kZeroPage,
    ZPX = ops::kZeroPageX,
    ZPY = ops::kZeroPageY,
    Abs = ops::kAbsolute,
    AbX = ops::kAbsoluteX,
    AbY = ops::kAbsoluteY,
    Ind = ops::kIndirect,
    InX = ops::kIndirectX,
    InY = ops::kIndirectY,
    Rel = ops::kRelative,

    XXX = ops::kUKN,
    ADC = ops::kADC,
    AND = ops::kAND,
    ASL = ops::kASL,
    BCC = ops::kBCC,
    BCS = ops::kBCS,
    BEQ = ops::kBEQ,
    BIT = ops::kBIT,
    BMI = ops::kBMI,
    BNE = ops::kBNE,
    BPL = ops::kBPL,
    BRK = ops::kBRK,
    BVC = ops::kBVC,
    BVS = ops::kBVS,
    CLC = ops::kCLC,
    CLD = ops::kCLD,
    CLI = ops::kCLI,
    CLV = ops::kCLV,
    CMP = ops::kCMP,
    CPX = ops::kCPX,
    CPY = ops::kCPY,
    DEC = ops::kDEC,
    DEX = ops::kDEX,
    DEY = ops::kDEY,
    EOR = ops::kEOR,
    INC = ops::kINC,
    INX = ops::kINX,
    INY = ops::kINY,
    JMP = ops::kJMP,
    JSR = ops::kJSR,
    LDA = ops::kLDA,
    LDX = ops::kLDX,
    LDY = ops::kLDY,
    LSR = ops::kLSR,
    NOP = ops::kNOP,
    ORA = ops::kORA,
    PHA = ops::kPHA,
    PHP = ops::kPHP,
    PLA = ops::kPLA,
    PLP = ops::kPLP,
    ROL = ops::kROL,
    ROR = ops::kROR,
    RTI = ops::kRTI,
    RTS = ops::kRTS,
    SBC = ops::kSBC,
    SEC = ops::kSEC,
    SED = ops::kSED,
    SEI = ops::kSEI,
    STA = ops::kSTA,
    STX = ops::kSTX,
    STY = ops::kSTY,
    TAX = ops::kTAX,
    TAY = ops::kTAY,
    TSX = ops::kTSX,
    TXA = ops::kTXA,
    TXS = ops::kTXS,
    TYA = ops::kTYA,

    DCP = ops::kDCP,
    ISB = ops::kISB,
    LAX = ops::kLAX,
    RLA = ops::kRLA,
    RRA = ops::kRRA,
    SAX = ops::kSAX,
    SLO = ops::kSLO,
    SRE = ops::kSRE,
};

const ops::metadata operation_data[] = {
    // OPCODEs from wiki.nesdev.wiki (mostly)
    // (Implemented) Unofficial opcodes are prefixed with an *

    // BRK(00)                ORA(01)                   ???(02)                   *SLO(03)                  *NOP(04)                  ORA(05)                   ASL(06)                   *SLO(07)
    {BRK, Non, 1, 7, " BRK"}, {ORA, InX, 2, 6, " ORA"}, {XXX, Non, 0, 2, "****"}, {SLO, InX, 2, 8, "*SLO"}, {NOP, ZPg, 2, 3, "*NOP"}, {ORA, ZPg, 2, 3, " ORA"}, {ASL, ZPg, 2, 5, " ASL"}, {SLO, ZPg, 2, 5, "*SLO"},
   
    // PHP(08)                ORA(09)                   ASL(0A)                   ???(0B)                   *NOP(0C)                  ORA(0D)                   ASL(0E)                   *SLO(0F)
    {PHP, Non, 1, 3, " PHP"}, {ORA, Imm, 2, 2, " ORA"}, {ASL, Non, 1, 2, " ASL"}, {XXX, Non, 0, 2, "****"}, {NOP, Abs, 3, 4, "*NOP"}, {ORA, Abs, 3, 4, " ORA"}, {ASL, Abs, 3, 6, " ASL"}, {SLO, Abs, 3, 6, "*SLO"},

    // BPL(10)                ORA(11)                   ???(12)                   *SLO(13)                  *NOP(14)                  ORA(15)                   ASL(16)                   *SLO(17)
    {BPL, Rel, 2, 2, " BPL"}, {ORA, InY, 2, 5, " ORA"}, {XXX, Non, 0, 2, "****"}, {SLO, InY, 2, 8, "*SLO"}, {NOP, ZPX, 2, 4, "*NOP"}, {ORA, ZPX, 2, 4, " ORA"}, {ASL, ZPX, 2, 6, " ASL"}, {SLO, ZPX, 2, 6, "*SLO"},

    // CLC(18)                ORA(19)                   *NOP(1A)                  *SLO(1B)                  *NOP(1C)                  ORA(1D)                   ASL(1E)                   *SLO(1F)
    {CLC, Non, 1, 2, " CLC"}, {ORA, AbY, 3, 4, " ORA"}, {NOP, Non, 1, 2, "*NOP"}, {SLO, AbY, 3, 7, "*SLO"}, {NOP, AbX, 3, 4, "*NOP"}, {ORA, AbX, 3, 4, " ORA"}, {ASL, AbX, 3, 7, " ASL"}, {SLO, AbX, 3, 7, "*SLO"},

    // JSR(20)                AND(21)                   ???(22)                   *RLA(23)                  BIT(24)                   AND(25)                   ROL(26)                   *RLA(27)
    {JSR, Abs, 3, 6, " JSR"}, {AND, InX, 2, 6, " AND"}, {XXX, Non, 0, 2, "****"}, {RLA, InX, 2, 8, "*RLA"}, {BIT, ZPg, 2, 3, " BIT"}, {AND, ZPg, 2, 3, " AND"}, {ROL, ZPg, 2, 5, " ROL"}, {RLA, ZPg, 2, 5, "*RLA"},

    // PLP(28)                AND(29)                   ROL(2A)                   ???(2B)                   BIT(2C)                   AND(2D)                   ROL(2E)                   *RLA(2F)
    {PLP, Non, 1, 4, " PLP"}, {AND, Imm, 2, 2, " AND"}, {ROL, Non, 1, 2, " ROL"}, {XXX, Non, 0, 2, "****"}, {BIT, Abs, 3, 4, " BIT"}, {AND, Abs, 3, 4, " AND"}, {ROL, Abs, 3, 6, " ROL"}, {RLA, Abs, 3, 6, "*RLA"},

    // BMI(30)                AND(31)                   ???(32)                   *RLA(33)                  *NOP(34)                  AND(35)                   ROL(36)                   *RLA(37)
    {BMI, Rel, 2, 2, " BMI"}, {AND, InY, 2, 5, " AND"}, {XXX, Non, 0, 2, "****"}, {RLA, InY, 2, 8, "*RLA"}, {NOP, ZPX, 2, 4, "*NOP"}, {AND, ZPX, 2, 4, " AND"}, {ROL, ZPX, 2, 6, " ROL"}, {RLA, ZPX, 2, 6, "*RLA"},

    // SEC(38)                AND(39)                   *NOP(3A)                  *RLA(3B)                  *NOP(3C)                  AND(3D)                   ROL(3E)                   *RLA(3F)
    {SEC, Non, 1, 2, " SEC"}, {AND, AbY, 3, 4, " AND"}, {NOP, Non, 1, 2, "*NOP"}, {RLA, AbY, 3, 7, "*RLA"}, {NOP, AbX, 3, 4, "*NOP"}, {AND, AbX, 3, 4, " AND"}, {ROL, AbX, 3, 7, " ROL"}, {RLA, AbX, 3, 7, "*RLA"},

    // RTI(40)                EOR(41)                   ???(42)                   *SRE(43)                  *NOP(44)                  EOR(45)                   LSR(46)                   *SRE(47)
    {RTI, Non, 1, 6, " RTI"}, {EOR, InX, 2, 6, " EOR"}, {XXX, Non, 0, 2, "****"}, {SRE, InX, 2, 8, "*SRE"}, {NOP, ZPg, 2, 3, "*NOP"}, {EOR, ZPg, 2, 3, " EOR"}, {LSR, ZPg, 2, 5, " LSR"}, {SRE, ZPg, 2, 5, "*SRE"},

    // PHA(48)                EOR(49)                   LSR(4A)                   ???(4B)                   JMP(4C)                   EOR(4D)                   LSR(4E)                   *SRE(4F)
    {PHA, Non, 1, 3, " PHA"}, {EOR, Imm, 2, 2, " EOR"}, {LSR, Non, 1, 2, " LSR"}, {XXX, Non, 0, 2, "****"}, {JMP, Abs, 3, 3, " JMP"}, {EOR, Abs, 3, 4, " EOR"}, {LSR, Abs, 3, 6, " LSR"}, {SRE, Abs, 3, 6, "*SRE"},

    // BVC(50)                EOR(51)                   ???(52)                   *SRE(53)                  *NOP(54)                  EOR(55)                   LSR(56)                   *SRE(57)
    {BVC, Rel, 2, 2, " BVC"}, {EOR, InY, 2, 5, " EOR"}, {XXX, Non, 0, 2, "****"}, {SRE, InY, 2, 8, "*SRE"}, {NOP, ZPX, 2, 4, "*NOP"}, {EOR, ZPX, 2, 4, " EOR"}, {LSR, ZPX, 2, 6, " LSR"}, {SRE, ZPX, 2, 6, "*SRE"},

    // CLI(58)                EOR(59)                   *NOP(5A)                  *SRE(5B)                  *NOP(5C)                  EOR(5D)                   LSR(5E)                   *SRE(5F)
    {CLI, Non, 1, 2, " CLI"}, {EOR, AbY, 3, 4, " EOR"}, {NOP, Non, 1, 2, "*NOP"}, {SRE, AbY, 3, 7, "*SRE"}, {NOP, AbX, 3, 4, "*NOP"}, {EOR, AbX, 3, 4, " EOR"}, {LSR, AbX, 3, 7, " LSR"}, {SRE, AbX, 3, 7, "*SRE"},

    // RTS(60)                ADC(61)                   ???(62)                   *RRA(63)                   *NOP(64)                  ADC(65)                   ROR(66)                   *RRA(67)
    {RTS, Non, 1, 6, " RTS"}, {ADC, InX, 2, 6, " ADC"}, {XXX, Non, 0, 2, "****"}, {RRA, InX, 2, 8, "*RRA"}, {NOP, ZPg, 2, 3, "*NOP"}, {ADC, ZPg, 2, 3, " ADC"}, {ROR, ZPg, 2, 5, " ROR"}, {RRA, ZPg, 2, 5, "*RRA"},

    // PLA(68)                ADC(69)                   ROR(6A)                   ???(6B)                   JMP(6C)                   ADC(6D)                   ROR(6E)                   *RRA(6F)
    {PLA, Non, 1, 4, " PLA"}, {ADC, Imm, 2, 2, " ADC"}, {ROR, Non, 1, 2, " ROR"}, {XXX, Non, 0, 2, "****"}, {JMP, Ind, 3, 5, " JMP"}, {ADC, Abs, 3, 4, " ADC"}, {ROR, Abs, 3, 6, " ROR"}, {RRA, Abs, 3, 6, "*RRA"},

    // BVS(70)                ADC(71)                   ???(72)                   *RRA(73)                  *NOP(74)                  ADC(75)                   ROR(76)                   *RRA(77)
    {BVS, Rel, 2, 2, " BVS"}, {ADC, InY, 2, 5, " ADC"}, {XXX, Non, 0, 2, "****"}, {RRA, InY, 2, 8, "*RRA"}, {NOP, ZPX, 2, 4, "*NOP"}, {ADC, ZPX, 2, 4, " ADC"}, {ROR, ZPX, 2, 6, " ROR"}, {RRA, ZPX, 2, 6, "*RRA"},

    // SEI(78)                ADC(79)                   *NOP(7A)                  *RRA(7B)                  *NOP(7C)                  ADC(7D)                   ROR(7E)                   *RRA(7F)
    {SEI, Non, 1, 2, " SEI"}, {ADC, AbY, 3, 4, " ADC"}, {NOP, Non, 1, 2, "*NOP"}, {RRA, AbY, 3, 7, "*RRA"}, {NOP, AbX, 3, 4, "*NOP"}, {ADC, AbX, 3, 4, " ADC"}, {ROR, AbX, 3, 7, " ROR"}, {RRA, AbX, 3, 7, "*RRA"},

    // *NOP(80)               STA(81)                   *NOP(82)                  *SAX(83)                  STY(84)                   STA(85)                   STX(86)                   *SAX(87)
    {NOP, Imm, 2, 2, "*NOP"}, {STA, InX, 2, 6, " STA"}, {NOP, Imm, 2, 2, "*NOP"}, {SAX, InX, 2, 6, "*SAX"}, {STY, ZPg, 2, 3, " STY"}, {STA, ZPg, 2, 3, " STA"}, {STX, ZPg, 2, 3, " STX"}, {SAX, ZPg, 2, 3, "*SAX"},

    // DEY(88)                *NOP(89)                   TXA(8A)                  ???(8B)                   STY(8C)                   STA(8D)                   STX(8E)                   *SAX(8F)
    {DEY, Non, 1, 2, " DEY"}, {NOP, Imm, 2, 2, "*NOP"}, {TXA, Non, 1, 2, " TXA"}, {XXX, Non, 0, 2, "****"}, {STY, Abs, 3, 4, " STY"}, {STA, Abs, 3, 4, " STA"}, {STX, Abs, 3, 4, " STX"}, {SAX, Abs, 3, 4, "*SAX"},

    // BCC(90)                STA(91)                   ???(92)                   ???(93)                   STY(94)                   STA(95)                   STX(96)                   *SAX(97)
    {BCC, Rel, 2, 2, " BCC"}, {STA, InY, 2, 6, " STA"}, {XXX, Non, 0, 2, "****"}, {XXX, Non, 0, 6, "****"}, {STY, ZPX, 2, 4, " STY"}, {STA, ZPX, 2, 4, " STA"}, {STX, ZPY, 2, 4, " STX"}, {SAX, ZPY, 2, 4, "*SAX"},

    // TYA(98)                STA(99)                   TXS(9A)                   ???(9B)                   ???(9C)                   STA(9D)                   ???(9E)                   ???(9F)
    {TYA, Non, 1, 2, " TYA"}, {STA, AbY, 3, 5, " STA"}, {TXS, Non, 1, 2, " TXS"}, {XXX, Non, 0, 5, "****"}, {XXX, Non, 0, 5, "****"}, {STA, AbX, 3, 5, " STA"}, {XXX, Non, 0, 5, "****"}, {XXX, Non, 0, 5, "****"},

    // LDY(A0)                LDA(A1)                   LDX(A2)                   *LAX(A3)                  LDY(A4)                   LDA(A5)                   LDX(A6)                   *LAX(A7)
    {LDY, Imm, 2, 2, " LDY"}, {LDA, InX, 2, 6, " LDA"}, {LDX, Imm, 2, 2, " LDX"}, {LAX, InX, 2, 6, "*LAX"}, {LDY, ZPg, 2, 3, " LDY"}, {LDA, ZPg, 2, 3, " LDA"}, {LDX, ZPg, 2, 3, " LDX"}, {LAX, ZPg, 2, 3, "*LAX"},

    // TAY(A8)                LDA(A9)                   TAX(AA)                   ???(AB)                   LDY(AC)                   LDA(AD)                   LDX(AE)                   *LAX(AF)
    {TAY, Non, 1, 2, " TAY"}, {LDA, Imm, 2, 2, " LDA"}, {TAX, Non, 1, 2, " TAX"}, {XXX, Non, 0, 2, "****"}, {LDY, Abs, 3, 4, " LDY"}, {LDA, Abs, 3, 4, " LDA"}, {LDX, Abs, 3, 4, " LDX"}, {LAX, Abs, 3, 4, "*LAX"},

    // BCS(B0)                LDA(B1)                   ???(B2)                   *LAX(B3)                  LDY(B4)                   LDA(B5)                   LDX(B6)                   *LAX(B7)
    {BCS, Rel, 2, 2, " BCS"}, {LDA, InY, 2, 5, " LDA"}, {XXX, Non, 0, 2, "****"}, {LAX, InY, 2, 5, "*LAX"}, {LDY, ZPX, 2, 4, " LDY"}, {LDA, ZPX, 2, 4, " LDA"}, {LDX, ZPY, 2, 4, " LDX"}, {LAX, ZPY, 2, 4, "*LAX"},

    // CLV(B8)                LDA(B9)                   TSX(BA)                   ???(BB)                   LDY(BC)                   LDA(BD)                   LDX(BE)                   *LAX(BF)
    {CLV, Non, 1, 2, " CLV"}, {LDA, AbY, 3, 4, " LDA"}, {TSX, Non, 1, 2, " TSX"}, {XXX, Non, 0, 4, "****"}, {LDY, AbX, 3, 4, " LDY"}, {LDA, AbX, 3, 4, " LDA"}, {LDX, AbY, 3, 4, " LDX"}, {LAX, AbY, 3, 4, "*LAX"},

    // CPY(C0)                CMP(C1)                   *NOP(C2)                  *DCP(C3)                   CPY(C4)                   CMP(C5)                   DEC(C6)                  *DCP(C7)
    {CPY, Imm, 2, 2, " CPY"}, {CMP, InX, 2, 6, " CMP"}, {NOP, Imm, 2, 2, "*NOP"}, {DCP, InX, 2, 8, "*DCP"}, {CPY, ZPg, 2, 3, " CPY"}, {CMP, ZPg, 2, 3, " CMP"}, {DEC, ZPg, 2, 5, " DEC"}, {DCP, ZPg, 2, 5, "*DCP"},

    // INY(C8)                CMP(C9)                   DEX(CA)                   ???(CB)                   CPY(CC)                   CMP(CD)                   DEC(CE)                   *DCP(CF)
    {INY, Non, 1, 2, " INY"}, {CMP, Imm, 2, 2, " CMP"}, {DEX, Non, 1, 2, " DEX"}, {XXX, Non, 0, 2, "****"}, {CPY, Abs, 3, 4, " CPY"}, {CMP, Abs, 3, 4, " CMP"}, {DEC, Abs, 3, 6, " DEC"}, {DCP, Abs, 3, 6, "*DCP"},

    // BNE(D0)                CMP(D1)                   ???(D2)                   *DCP(D3)                  *NOP(D4)                  CMP(D5)                  DEC(D6)                    *DCP(D7)
    {BNE, Rel, 2, 2, " BNE"}, {CMP, InY, 2, 5, " CMP"}, {XXX, Non, 0, 2, "****"}, {DCP, InY, 2, 8, "*DCP"}, {NOP, ZPX, 2, 4, "*NOP"}, {CMP, ZPX, 2, 4, " CMP"}, {DEC, ZPX, 2, 6, " DEC"}, {DCP, ZPX, 2, 6, "*DCP"},

    // CLD(D8)                CMP(D9)                   *NOP(DA)                  *DCP(DB)                  *NOP(DC)                  CMP(DD)                  DEC(DE)                    *DCP(DF)
    {CLD, Non, 1, 2, " CLD"}, {CMP, AbY, 3, 4, " CMP"}, {NOP, Non, 1, 2, "*NOP"}, {DCP, AbY, 3, 7, "*DCP"}, {NOP, AbX, 3, 4, "*NOP"}, {CMP, AbX, 3, 4, " CMP"}, {DEC, AbX, 3, 7, " DEC"}, {DCP, AbX, 3, 7, "*DCP"},

    // CPX(E0)                SBC(E1)                   ???(E2)                   *ISB(E3)                  CPX(E4)                   SBC(E5)                   INC(E6)                   *ISB(E7)
    {CPX, Imm, 2, 2, " CPX"}, {SBC, InX, 2, 6, " SBC"}, {XXX, Non, 0, 2, "****"}, {ISB, InX, 2, 8, "*ISB"}, {CPX, ZPg, 2, 3, " CPX"}, {SBC, ZPg, 2, 3, " SBC"}, {INC, ZPg, 2, 5, " INC"}, {ISB, ZPg, 2, 5, "*ISB"},

    // INX(E8)                SBC(E9)                   NOP(EA)                   *SBC(EB)                  CPX(EC)                   SBC(ED)                   INC(EE)                   *ISB(EF)
    {INX, Non, 1, 2, " INX"}, {SBC, Imm, 2, 2, " SBC"}, {NOP, Non, 1, 2, " NOP"}, {SBC, Imm, 2, 2, "*SBC"}, {CPX, Abs, 3, 4, " CPX"}, {SBC, Abs, 3, 4, " SBC"}, {INC, Abs, 3, 6, " INC"}, {ISB, Abs, 3, 6, "*ISB"},

    // BEQ(F0)                SBC(F1)                   ???(F2)                   *ISB(F3)                  *NOP(F4)                  SBC(F5)                   INC(F6)                   *ISB(F7)
    {BEQ, Rel, 2, 2, " BEQ"}, {SBC, InY, 2, 5, " SBC"}, {XXX, Non, 0, 2, "****"}, {ISB, InY, 2, 8, "*ISB"}, {NOP, ZPX, 2, 4, "*NOP"}, {SBC, ZPX, 2, 4, " SBC"}, {INC, ZPX, 2, 6, " INC"}, {ISB, ZPX, 2, 6, "*ISB"},

    // SED(F8)                SBC(F9)                   *NOP(FA)                  *ISB(FB)                  *NOP(FC)                  SBC(FD)                   INC(FE)                   *ISB(FF)
    {SED, Non, 1, 2, " SED"}, {SBC, AbY, 3, 4, " SBC"}, {NOP, Non, 1, 2, "*NOP"}, {ISB, AbY, 3, 7, "*ISB"}, {NOP, AbX, 3, 4, "*NOP"}, {SBC, AbX, 3, 4, " SBC"}, {INC, AbX, 3, 7, " INC"}, {ISB, AbX, 3, 7, "*ISB"}};
}

ops::metadata ops::opcode_data(byte_t opcode)
{
    return operation_data[opcode];
}
