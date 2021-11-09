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

    ALR = ops::kNOP,
    ANC = ops::kNOP,
    ANE = ops::kNOP,
    ARR = ops::kNOP,
    DCP = ops::kDCP,
    ISB = ops::kISB,
    ISC = ops::kNOP,
    LAS = ops::kNOP,
    LAX = ops::kLAX,
    LXA = ops::kNOP,
    RLA = ops::kRLA,
    RRA = ops::kRRA,
    SAX = ops::kSAX,
    SBX = ops::kNOP,
    SHA = ops::kNOP,
    SHX = ops::kNOP,
    SHY = ops::kNOP,
    SLO = ops::kSLO,
    SRE = ops::kSRE,
    TAS = ops::kNOP,
};

const ops::metadata operation_data[] = {
    // OPCODEs data from wiki.nesdev.org and masswerk.at/6502
    // (Implemented) Unofficial opcodes are prefixed with an *

    // BRK(00)             ORA(01)                ???(02)                *SLO(03)               *NOP(04)               ORA(05)                ASL(06)                *SLO(07)
    {BRK, Non, 7, " BRK"}, {ORA, InX, 6, " ORA"}, {XXX, Non, 2, "****"}, {SLO, InX, 8, "*SLO"}, {NOP, ZPg, 3, "*NOP"}, {ORA, ZPg, 3, " ORA"}, {ASL, ZPg, 5, " ASL"}, {SLO, ZPg, 5, "*SLO"},
   
    // PHP(08)             ORA(09)                ASL(0A)                *ANC(0B)               *NOP(0C)               ORA(0D)                ASL(0E)                *SLO(0F)
    {PHP, Non, 3, " PHP"}, {ORA, Imm, 2, " ORA"}, {ASL, Non, 2, " ASL"}, {ANC, Imm, 2, "*ANC"}, {NOP, Abs, 4, "*NOP"}, {ORA, Abs, 4, " ORA"}, {ASL, Abs, 6, " ASL"}, {SLO, Abs, 6, "*SLO"},

    // BPL(10)             ORA(11)                ???(12)                *SLO(13)               *NOP(14)               ORA(15)                ASL(16)                *SLO(17)
    {BPL, Rel, 2, " BPL"}, {ORA, InY, 5, " ORA"}, {XXX, Non, 2, "****"}, {SLO, InY, 8, "*SLO"}, {NOP, ZPX, 4, "*NOP"}, {ORA, ZPX, 4, " ORA"}, {ASL, ZPX, 6, " ASL"}, {SLO, ZPX, 6, "*SLO"},

    // CLC(18)             ORA(19)                *NOP(1A)               *SLO(1B)               *NOP(1C)               ORA(1D)                ASL(1E)                *SLO(1F)
    {CLC, Non, 2, " CLC"}, {ORA, AbY, 4, " ORA"}, {NOP, Non, 2, "*NOP"}, {SLO, AbY, 7, "*SLO"}, {NOP, AbX, 4, "*NOP"}, {ORA, AbX, 4, " ORA"}, {ASL, AbX, 7, " ASL"}, {SLO, AbX, 7, "*SLO"},

    // JSR(20)             AND(21)                ???(22)                *RLA(23)               BIT(24)                AND(25)                ROL(26)                *RLA(27)
    {JSR, Abs, 6, " JSR"}, {AND, InX, 6, " AND"}, {XXX, Non, 2, "****"}, {RLA, InX, 8, "*RLA"}, {BIT, ZPg, 3, " BIT"}, {AND, ZPg, 3, " AND"}, {ROL, ZPg, 5, " ROL"}, {RLA, ZPg, 5, "*RLA"},

    // PLP(28)             AND(29)                ROL(2A)                *ANC(2B)               BIT(2C)                AND(2D)                ROL(2E)                *RLA(2F)
    {PLP, Non, 4, " PLP"}, {AND, Imm, 2, " AND"}, {ROL, Non, 2, " ROL"}, {ANC, Imm, 2, "*ANC"}, {BIT, Abs, 4, " BIT"}, {AND, Abs, 4, " AND"}, {ROL, Abs, 6, " ROL"}, {RLA, Abs, 6, "*RLA"},

    // BMI(30)             AND(31)                ???(32)                *RLA(33)               *NOP(34)               AND(35)                ROL(36)                *RLA(37)
    {BMI, Rel, 2, " BMI"}, {AND, InY, 5, " AND"}, {XXX, Non, 2, "****"}, {RLA, InY, 8, "*RLA"}, {NOP, ZPX, 4, "*NOP"}, {AND, ZPX, 4, " AND"}, {ROL, ZPX, 6, " ROL"}, {RLA, ZPX, 6, "*RLA"},

    // SEC(38)             AND(39)                *NOP(3A)               *RLA(3B)               *NOP(3C)               AND(3D)                ROL(3E)                *RLA(3F)
    {SEC, Non, 2, " SEC"}, {AND, AbY, 4, " AND"}, {NOP, Non, 2, "*NOP"}, {RLA, AbY, 7, "*RLA"}, {NOP, AbX, 4, "*NOP"}, {AND, AbX, 4, " AND"}, {ROL, AbX, 7, " ROL"}, {RLA, AbX, 7, "*RLA"},

    // RTI(40)             EOR(41)                ???(42)                *SRE(43)               *NOP(44)               EOR(45)                LSR(46)                *SRE(47)
    {RTI, Non, 6, " RTI"}, {EOR, InX, 6, " EOR"}, {XXX, Non, 2, "****"}, {SRE, InX, 8, "*SRE"}, {NOP, ZPg, 3, "*NOP"}, {EOR, ZPg, 3, " EOR"}, {LSR, ZPg, 5, " LSR"}, {SRE, ZPg, 5, "*SRE"},

    // PHA(48)             EOR(49)                LSR(4A)                *ALR(4B)               JMP(4C)                EOR(4D)                LSR(4E)                *SRE(4F)
    {PHA, Non, 3, " PHA"}, {EOR, Imm, 2, " EOR"}, {LSR, Non, 2, " LSR"}, {ALR, Imm, 2, "*ALR"}, {JMP, Abs, 3, " JMP"}, {EOR, Abs, 4, " EOR"}, {LSR, Abs, 6, " LSR"}, {SRE, Abs, 6, "*SRE"},

    // BVC(50)             EOR(51)                ???(52)                *SRE(53)               *NOP(54)               EOR(55)                LSR(56)                *SRE(57)
    {BVC, Rel, 2, " BVC"}, {EOR, InY, 5, " EOR"}, {XXX, Non, 2, "****"}, {SRE, InY, 8, "*SRE"}, {NOP, ZPX, 4, "*NOP"}, {EOR, ZPX, 4, " EOR"}, {LSR, ZPX, 6, " LSR"}, {SRE, ZPX, 6, "*SRE"},

    // CLI(58)             EOR(59)                *NOP(5A)               *SRE(5B)               *NOP(5C)               EOR(5D)                LSR(5E)                *SRE(5F)
    {CLI, Non, 2, " CLI"}, {EOR, AbY, 4, " EOR"}, {NOP, Non, 2, "*NOP"}, {SRE, AbY, 7, "*SRE"}, {NOP, AbX, 4, "*NOP"}, {EOR, AbX, 4, " EOR"}, {LSR, AbX, 7, " LSR"}, {SRE, AbX, 7, "*SRE"},

    // RTS(60)             ADC(61)                ???(62)                *RRA(63)               *NOP(64)               ADC(65)                ROR(66)                *RRA(67)
    {RTS, Non, 6, " RTS"}, {ADC, InX, 6, " ADC"}, {XXX, Non, 2, "****"}, {RRA, InX, 8, "*RRA"}, {NOP, ZPg, 3, "*NOP"}, {ADC, ZPg, 3, " ADC"}, {ROR, ZPg, 5, " ROR"}, {RRA, ZPg, 5, "*RRA"},

    // PLA(68)             ADC(69)                ROR(6A)                *ARR(6B)               JMP(6C)                ADC(6D)                ROR(6E)                *RRA(6F)
    {PLA, Non, 4, " PLA"}, {ADC, Imm, 2, " ADC"}, {ROR, Non, 2, " ROR"}, {ARR, Imm, 2, "*ARR"}, {JMP, Ind, 5, " JMP"}, {ADC, Abs, 4, " ADC"}, {ROR, Abs, 6, " ROR"}, {RRA, Abs, 6, "*RRA"},

    // BVS(70)             ADC(71)                ???(72)                *RRA(73)               *NOP(74)               ADC(75)                ROR(76)                *RRA(77)
    {BVS, Rel, 2, " BVS"}, {ADC, InY, 5, " ADC"}, {XXX, Non, 2, "****"}, {RRA, InY, 8, "*RRA"}, {NOP, ZPX, 4, "*NOP"}, {ADC, ZPX, 4, " ADC"}, {ROR, ZPX, 6, " ROR"}, {RRA, ZPX, 6, "*RRA"},

    // SEI(78)             ADC(79)                *NOP(7A)               *RRA(7B)               *NOP(7C)               ADC(7D)                ROR(7E)                *RRA(7F)
    {SEI, Non, 2, " SEI"}, {ADC, AbY, 4, " ADC"}, {NOP, Non, 2, "*NOP"}, {RRA, AbY, 7, "*RRA"}, {NOP, AbX, 4, "*NOP"}, {ADC, AbX, 4, " ADC"}, {ROR, AbX, 7, " ROR"}, {RRA, AbX, 7, "*RRA"},

    // *NOP(80)            STA(81)                *NOP(82)               *SAX(83)               STY(84)                STA(85)                STX(86)                *SAX(87)
    {NOP, Imm, 2, "*NOP"}, {STA, InX, 6, " STA"}, {NOP, Imm, 2, "*NOP"}, {SAX, InX, 6, "*SAX"}, {STY, ZPg, 3, " STY"}, {STA, ZPg, 3, " STA"}, {STX, ZPg, 3, " STX"}, {SAX, ZPg, 3, "*SAX"},

    // DEY(88)             *NOP(89)                TXA(8A)               *ANE(8B)               STY(8C)                STA(8D)                STX(8E)                *SAX(8F)
    {DEY, Non, 2, " DEY"}, {NOP, Imm, 2, "*NOP"}, {TXA, Non, 2, " TXA"}, {ANE, Imm, 2, "*ANE"}, {STY, Abs, 4, " STY"}, {STA, Abs, 4, " STA"}, {STX, Abs, 4, " STX"}, {SAX, Abs, 4, "*SAX"},

    // BCC(90)             STA(91)                ???(92)                *SHA(93)               STY(94)                STA(95)                STX(96)                *SAX(97)
    {BCC, Rel, 2, " BCC"}, {STA, InY, 6, " STA"}, {XXX, Non, 2, "****"}, {SHA, InY, 6, "*SHA"}, {STY, ZPX, 4, " STY"}, {STA, ZPX, 4, " STA"}, {STX, ZPY, 4, " STX"}, {SAX, ZPY, 4, "*SAX"},

    // TYA(98)             STA(99)                TXS(9A)                *TAS(9B)               *SHY(9C)               STA(9D)                *SHX(9E)               *SHA(9F)
    {TYA, Non, 2, " TYA"}, {STA, AbY, 5, " STA"}, {TXS, Non, 2, " TXS"}, {TAS, AbY, 5, "*TAS"}, {SHY, AbX, 5, "*SHY"}, {STA, AbX, 5, " STA"}, {SHX, AbY, 5, "*SHX"}, {SHA, AbY, 5, "*SHA"},

    // LDY(A0)             LDA(A1)                LDX(A2)                *LAX(A3)               LDY(A4)                LDA(A5)                LDX(A6)                *LAX(A7)
    {LDY, Imm, 2, " LDY"}, {LDA, InX, 6, " LDA"}, {LDX, Imm, 2, " LDX"}, {LAX, InX, 6, "*LAX"}, {LDY, ZPg, 3, " LDY"}, {LDA, ZPg, 3, " LDA"}, {LDX, ZPg, 3, " LDX"}, {LAX, ZPg, 3, "*LAX"},

    // TAY(A8)             LDA(A9)                TAX(AA)                *LXA(AB)               LDY(AC)                LDA(AD)                LDX(AE)                *LAX(AF)
    {TAY, Non, 2, " TAY"}, {LDA, Imm, 2, " LDA"}, {TAX, Non, 2, " TAX"}, {LXA, Imm, 2, "*LXA"}, {LDY, Abs, 4, " LDY"}, {LDA, Abs, 4, " LDA"}, {LDX, Abs, 4, " LDX"}, {LAX, Abs, 4, "*LAX"},

    // BCS(B0)             LDA(B1)                ???(B2)                *LAX(B3)               LDY(B4)                LDA(B5)                LDX(B6)                *LAX(B7)
    {BCS, Rel, 2, " BCS"}, {LDA, InY, 5, " LDA"}, {XXX, Non, 2, "****"}, {LAX, InY, 5, "*LAX"}, {LDY, ZPX, 4, " LDY"}, {LDA, ZPX, 4, " LDA"}, {LDX, ZPY, 4, " LDX"}, {LAX, ZPY, 4, "*LAX"},

    // CLV(B8)             LDA(B9)                TSX(BA)                *LAS(BB)               LDY(BC)                LDA(BD)                LDX(BE)                *LAX(BF)
    {CLV, Non, 2, " CLV"}, {LDA, AbY, 4, " LDA"}, {TSX, Non, 2, " TSX"}, {LAS, AbY, 4, "*LAS"}, {LDY, AbX, 4, " LDY"}, {LDA, AbX, 4, " LDA"}, {LDX, AbY, 4, " LDX"}, {LAX, AbY, 4, "*LAX"},

    // CPY(C0)             CMP(C1)                *NOP(C2)               *DCP(C3)               CPY(C4)                CMP(C5)                DEC(C6)                *DCP(C7)
    {CPY, Imm, 2, " CPY"}, {CMP, InX, 6, " CMP"}, {NOP, Imm, 2, "*NOP"}, {DCP, InX, 8, "*DCP"}, {CPY, ZPg, 3, " CPY"}, {CMP, ZPg, 3, " CMP"}, {DEC, ZPg, 5, " DEC"}, {DCP, ZPg, 5, "*DCP"},

    // INY(C8)             CMP(C9)                DEX(CA)                *SBX(CB)               CPY(CC)                CMP(CD)                DEC(CE)                *DCP(CF)
    {INY, Non, 2, " INY"}, {CMP, Imm, 2, " CMP"}, {DEX, Non, 2, " DEX"}, {SBX, Imm, 2, "*SBX"}, {CPY, Abs, 4, " CPY"}, {CMP, Abs, 4, " CMP"}, {DEC, Abs, 6, " DEC"}, {DCP, Abs, 6, "*DCP"},

    // BNE(D0)             CMP(D1)                ???(D2)                *DCP(D3)               *NOP(D4)               CMP(D5)                DEC(D6)                *DCP(D7)
    {BNE, Rel, 2, " BNE"}, {CMP, InY, 5, " CMP"}, {XXX, Non, 2, "****"}, {DCP, InY, 8, "*DCP"}, {NOP, ZPX, 4, "*NOP"}, {CMP, ZPX, 4, " CMP"}, {DEC, ZPX, 6, " DEC"}, {DCP, ZPX, 6, "*DCP"},

    // CLD(D8)             CMP(D9)                *NOP(DA)               *DCP(DB)               *NOP(DC)               CMP(DD)                DEC(DE)                *DCP(DF)
    {CLD, Non, 2, " CLD"}, {CMP, AbY, 4, " CMP"}, {NOP, Non, 2, "*NOP"}, {DCP, AbY, 7, "*DCP"}, {NOP, AbX, 4, "*NOP"}, {CMP, AbX, 4, " CMP"}, {DEC, AbX, 7, " DEC"}, {DCP, AbX, 7, "*DCP"},

    // CPX(E0)             SBC(E1)                *NOP(E2)               *ISB(E3)               CPX(E4)                SBC(E5)                INC(E6)                *ISB(E7)
    {CPX, Imm, 2, " CPX"}, {SBC, InX, 6, " SBC"}, {NOP, Imm, 2, "*NOP"}, {ISB, InX, 8, "*ISB"}, {CPX, ZPg, 3, " CPX"}, {SBC, ZPg, 3, " SBC"}, {INC, ZPg, 5, " INC"}, {ISB, ZPg, 5, "*ISB"},

    // INX(E8)             SBC(E9)                NOP(EA)                *SBC(EB)               CPX(EC)                SBC(ED)                INC(EE)                *ISB(EF)
    {INX, Non, 2, " INX"}, {SBC, Imm, 2, " SBC"}, {NOP, Non, 2, " NOP"}, {SBC, Imm, 2, "*SBC"}, {CPX, Abs, 4, " CPX"}, {SBC, Abs, 4, " SBC"}, {INC, Abs, 6, " INC"}, {ISB, Abs, 6, "*ISB"},

    // BEQ(F0)             SBC(F1)                ???(F2)                *ISB(F3)               *NOP(F4)               SBC(F5)                INC(F6)                *ISB(F7)
    {BEQ, Rel, 2, " BEQ"}, {SBC, InY, 5, " SBC"}, {XXX, Non, 2, "****"}, {ISB, InY, 8, "*ISB"}, {NOP, ZPX, 4, "*NOP"}, {SBC, ZPX, 4, " SBC"}, {INC, ZPX, 6, " INC"}, {ISB, ZPX, 6, "*ISB"},

    // SED(F8)             SBC(F9)                *NOP(FA)               *ISB(FB)               *NOP(FC)               SBC(FD)                INC(FE)                *ISB(FF)
    {SED, Non, 2, " SED"}, {SBC, AbY, 4, " SBC"}, {NOP, Non, 2, "*NOP"}, {ISB, AbY, 7, "*ISB"}, {NOP, AbX, 4, "*NOP"}, {SBC, AbX, 4, " SBC"}, {INC, AbX, 7, " INC"}, {ISB, AbX, 7, "*ISB"}};
}

const ops::metadata& ops::opcode_data(byte_t opcode)
{
    return operation_data[opcode];
}

byte_t ops::metadata::get_size() const
{
    switch (addressing)
    {
    case kNone:
        return 1;

    case kImmediate:
    case kZeroPage:
    case kZeroPageX:
    case kZeroPageY:
    case kIndirectX:
    case kIndirectY:
    case kRelative:
        return 2;

    case kAbsolute:
    case kAbsoluteX:
    case kAbsoluteY:
    case kIndirect:
        return 3;
    }

    return 0;
}
