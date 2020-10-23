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
    Rel = ops::kRelative
};

const ops::metadata operation_data[] = {
    // OPCODEs from wiki.nesdev.wiki (mostly)
    // (Implemented) Unofficial opcodes are prefixed with an *

    // TODO: Unofficial opcodes are missing timings

    // BRK(00)           ORA(01)              ???(02)              ???(03)              *NOP(04)              ORA(05)              ASL(06)              ???(07)
    {1, 7, Non, " BRK"}, {2, 6, InX, " ORA"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPg, "*NOP"}, {2, 3, ZPg, " ORA"}, {2, 5, ZPg, " ASL"}, {0, 0, Non, "****"},
   
    // PHP(08)           ORA(09)              ASL(0A)              ???(0B)              *NOP(0C)              ORA(0D)              ASL(0E)              ???(0F)
    {1, 3, Non, " PHP"}, {2, 2, Imm, " ORA"}, {1, 2, Non, " ASL"}, {0, 0, Non, "****"}, {3, 0, Abs, "*NOP"}, {3, 4, Abs, " ORA"}, {3, 6, Abs, " ASL"}, {0, 0, Non, "****"},

    // BPL(10)           ORA(11)              ???(12)              ???(13)              *NOP(14)             ORA(15)              ASL(16)              ???(17)
    {2, 2, Rel, " BPL"}, {2, 5, InY, " ORA"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPX, "*NOP"}, {2, 4, ZPX, " ORA"}, {2, 6, ZPX, " ASL"}, {0, 0, Non, "****"},

    // CLC(18)           ORA(19)              *NOP(1A)             ???(1B)              *NOP(1C)             ORA(1D)              ASL(1E)              ???(1F)
    {1, 2, Non, " CLC"}, {3, 4, AbY, " ORA"}, {1, 0, Non, "*NOP"}, {0, 0, Non, "****"}, {3, 0, AbX, "*NOP"}, {3, 4, AbX, " ORA"}, {3, 7, AbX, " ASL"}, {0, 0, Non, "****"},

    // JSR(20)           AND(21)              ???(22)              ???(23)              BIT(24)              AND(25)              ROL(26)              ???(27)
    {3, 6, Abs, " JSR"}, {2, 6, InX, " AND"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 3, ZPg, " BIT"}, {2, 3, ZPg, " AND"}, {2, 5, ZPg, " ROL"}, {0, 0, Non, "****"},

    // PLP(28)           AND(29)              ROL(2A)              ???(2B)              BIT(2C)              AND(2D)              ROL(2E)              ???(2F)
    {1, 4, Non, " PLP"}, {2, 2, Imm, " AND"}, {1, 2, Non, " ROL"}, {0, 0, Non, "****"}, {3, 4, Abs, " BIT"}, {3, 4, Abs, " AND"}, {3, 6, Abs, " ROL"}, {0, 0, Non, "****"},

    // BMI(30)           AND(31)              ???(32)              ???(33)              *NOP(34)             AND(35)              ROL(36)              ???(37)
    {2, 2, Rel, " BMI"}, {2, 5, InY, " AND"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPX, "*NOP"}, {2, 4, ZPX, " AND"}, {2, 6, ZPX, " ROL"}, {0, 0, Non, "****"},

    // SEC(38)           AND(39)              *NOP(3A)             ???(3B)              *NOP(3C)             AND(3D)              ROL(3E)              ???(3F)
    {1, 2, Non, " SEC"}, {3, 4, AbY, " AND"}, {1, 0, Non, "*NOP"}, {0, 0, Non, "****"}, {3, 0, AbX, "*NOP"}, {3, 4, AbX, " AND"}, {3, 7, AbX, " ROL"}, {0, 0, Non, "****"},

    // RTI(40)           EOR(41)              ???(42)              ???(43)              *NOP(44)             EOR(45)              LSR(46)              ???(47)
    {1, 6, Non, " RTI"}, {2, 6, InX, " EOR"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPg, "*NOP"}, {2, 3, ZPg, " EOR"}, {2, 5, ZPg, " LSR"}, {0, 0, Non, "****"},

    // PHA(48)           EOR(49)              LSR(4A)              ???(4B)              JMP(4C)              EOR(4D)              LSR(4E)              ???(4F)
    {1, 3, Non, " PHA"}, {2, 2, Imm, " EOR"}, {1, 2, Non, " LSR"}, {0, 0, Non, "****"}, {3, 3, Abs, " JMP"}, {3, 4, Abs, " EOR"}, {3, 6, Abs, " LSR"}, {0, 0, Non, "****"},

    // BVC(50)           EOR(51)              ???(52)              ???(53)              *NOP(54)             EOR(55)              LSR(56)              ???(57)
    {2, 2, Rel, " BVC"}, {2, 5, InY, " EOR"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPX, "*NOP"}, {2, 4, ZPX, " EOR"}, {2, 6, ZPX, " LSR"}, {0, 0, Non, "****"},

    // CLI(58)           EOR(59)              *NOP(5A)             ???(5B)              *NOP(5C)             EOR(5D)              LSR(5E)              ???(5F)
    {1, 2, Non, " CLI"}, {3, 4, AbY, " EOR"}, {1, 0, Non, "*NOP"}, {0, 0, Non, "****"}, {3, 0, AbX, "*NOP"}, {3, 4, AbX, " EOR"}, {3, 7, AbX, " LSR"}, {0, 0, Non, "****"},

    // RTS(60)           ADC(61)              ???(62)              ???(63)              *NOP(64)             ADC(65)              ROR(66)              ???(67)
    {1, 6, Non, " RTS"}, {2, 6, InX, " ADC"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPg, "*NOP"}, {2, 3, ZPg, " ADC"}, {2, 5, ZPg, " ROR"}, {0, 0, Non, "****"},

    // PLA(68)           ADC(69)              ROR(6A)              ???(6B)              JMP(6C)              ADC(6D)              ROR(6E)              ???(6F)
    {1, 4, Non, " PLA"}, {2, 2, Imm, " ADC"}, {1, 2, Non, " ROR"}, {0, 0, Non, "****"}, {3, 5, Ind, " JMP"}, {3, 4, Abs, " ADC"}, {3, 6, Abs, " ROR"}, {0, 0, Non, "****"},

    // BVS(70)           ADC(71)              ???(72)              ???(73)              *NOP(74)             ADC(75)              ROR(76)              ???(77)
    {2, 2, Rel, " BVS"}, {2, 5, InX, " ADC"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPX, "*NOP"}, {2, 4, ZPX, " ADC"}, {2, 6, ZPX, " ROR"}, {0, 0, Non, "****"},

    // SEI(78)           ADC(79)              *NOP(7A)             ???(7B)              *NOP(7C)             ADC(7D)              ROR(7E)              ???(7F)
    {1, 2, Non, " SEI"}, {3, 4, AbY, " ADC"}, {1, 0, Non, "*NOP"}, {0, 0, Non, "****"}, {3, 0, AbX, "*NOP"}, {3, 4, AbX, " ADC"}, {3, 7, AbX, " ROR"}, {0, 0, Non, "****"},

    // *NOP(80)          STA(81)              *NOP(82)             *SAX(83)             STY(84)              STA(85)              STX(86)              *SAX(87)
    {2, 0, Imm, "*NOP"}, {2, 6, InX, " STA"}, {2, 0, Imm, "*NOP"}, {2, 0, InX, "*SAX"}, {2, 3, ZPg, " STY"}, {2, 3, ZPg, " STA"}, {2, 3, ZPg, " STX"}, {2, 0, ZPg, "*SAX"},

    // DEY(88)           *NOP(89)              TXA(8A)             ???(8B)              STY(8C)              STA(8D)              STX(8E)              *SAX(8F)
    {1, 2, Non, " DEY"}, {2, 0, Imm, "*NOP"}, {1, 2, Non, " TXA"}, {0, 0, Non, "****"}, {3, 4, Abs, " STY"}, {3, 4, Abs, " STA"}, {3, 4, Abs, " STX"}, {3, 0, Abs, "*SAX"},

    // BCC(90)           STA(91)              ???(92)              ???(93)              STY(94)              STA(95)              STX(96)              *SAX(97)
    {2, 2, Rel, " BCC"}, {2, 6, InY, " STA"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 4, ZPX, " STY"}, {2, 4, ZPX, " STA"}, {2, 4, ZPY, " STX"}, {2, 0, ZPY, "*SAX"},

    // TYA(98)           STA(99)              TXS(9A)              ???(9B)              ???(9C)              STA(9D)              ???(9E)              ???(9F)
    {1, 0, Non, " TYA"}, {3, 5, AbY, " STA"}, {1, 2, Non, " TXS"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {3, 5, AbX, " STA"}, {0, 0, Non, "****"}, {0, 0, Non, "****"},

    // LDY(A0)           LDA(A1)              LDX(A2)              *LAX(A3)             LDY(A4)              LDA(A5)              LDX(A6)              *LAX(A7)
    {2, 2, Imm, " LDY"}, {2, 6, InX, " LDA"}, {2, 2, Imm, " LDX"}, {2, 0, InX, "*LAX"}, {2, 3, ZPg, " LDY"}, {2, 3, ZPg, " LDA"}, {2, 3, ZPg, " LDX"}, {2, 0, ZPg, "*LAX"},

    // TAY(A8)           LDA(A9)              TAX(AA)              ???(AB)              LDY(AC)              LDA(AD)              LDX(AE)              *LAX(AF)
    {1, 2, Non, " TAY"}, {2, 2, Imm, " LDA"}, {1, 2, Non, " TAX"}, {0, 0, Non, "****"}, {3, 4, Abs, " LDY"}, {3, 4, Abs, " LDA"}, {3, 4, Abs, " LDX"}, {3, 0, Abs, "*LAX"},

    // BCS(B0)           LDA(B1)              ???(B2)              *LAX(B3)             LDY(B4)              LDA(B5)              LDX(B6)              *LAX(B7)
    {2, 2, Rel, " BCS"}, {2, 5, InY, " LDA"}, {0, 0, Non, "****"}, {2, 0, InY, "*LAX"}, {2, 4, ZPX, " LDY"}, {2, 4, ZPX, " LDA"}, {2, 4, ZPY, " LDX"}, {2, 0, ZPY, "*LAX"},

    // CLV(B8)           LDA(B9)              TSX(BA)              ???(BB)              LDY(BC)              LDA(BD)              LDX(BE)              *LAX(BF)
    {1, 2, Non, " CLV"}, {3, 4, AbY, " LDA"}, {1, 2, Non, " TSX"}, {0, 0, Non, "****"}, {3, 4, AbX, " LDY"}, {3, 4, AbX, " LDA"}, {3, 4, AbY, " LDX"}, {3, 0, AbY, "*LAX"},

    // CPY(C0)           CMP(C1)              ???(C2)              ???(C3)              CPY(C4)              CMP(C5)              DEC(C6)              ???(C7)
    {2, 2, Imm, " CPY"}, {2, 6, InX, " CMP"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 3, ZPg, " CPY"}, {2, 3, ZPg, " CMP"}, {2, 5, ZPg, " DEC"}, {0, 0, Non, "****"},

    // INY(C8)           CMP(C9)              DEX(CA)              ???(CB)              CPY(CC)              CMP(CD)              DEC(CE)              ???(CF)
    {1, 2, Non, " INY"}, {2, 2, Imm, " CMP"}, {1, 2, Non, " DEX"}, {0, 0, Non, "****"}, {3, 4, Abs, " CPY"}, {3, 4, Abs, " CMP"}, {3, 6, Abs, " DEC"}, {0, 0, Non, "****"},

    // BNE(D0)           CMP(D1)              ???(D2)              ???(D3)              *NOP(D4)             CMP(D5)              DEC(D6)              ???(D7)
    {2, 2, Rel, " BNE"}, {2, 5, InY, " CMP"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPX, "*NOP"}, {2, 4, ZPX, " CMP"}, {2, 6, ZPX, " DEC"}, {0, 0, Non, "****"},

    // CLD(D8)           CMP(D9)              *NOP(DA)             ???(DB)              *NOP(DC)             CMP(DD)              DEC(DE)              ???(DF)
    {1, 2, Non, " CLD"}, {3, 4, AbY, " CMP"}, {1, 0, Non, "*NOP"}, {0, 0, Non, "****"}, {3, 0, AbX, "*NOP"}, {3, 4, AbX, " CMP"}, {3, 7, AbX, " DEC"}, {0, 0, Non, "****"},

    // CPX(E0)           SBC(E1)              ???(E2)              ???(E3)              CPX(E4)              SBC(E5)              INC(E6)              ???(E7)
    {2, 2, Imm, " CPX"}, {2, 6, InX, " SBC"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 3, ZPg, " CPX"}, {2, 3, ZPg, " SBC"}, {2, 5, ZPg, " INC"}, {0, 0, Non, "****"},

    // INX(E8)           SBC(E9)              NOP(EA)              ???(EB)              CPX(EC)              SBC(ED)              INC(EE)              ???(EF)
    {1, 2, Non, " INX"}, {2, 2, Imm, " SBC"}, {1, 2, Non, " NOP"}, {0, 0, Non, "****"}, {3, 4, Abs, " CPX"}, {3, 4, Abs, " SBC"}, {3, 6, Abs, " INC"}, {0, 0, Non, "****"},

    // BEQ(F0)           SBC(F1)              ???(F2)              ???(F3)              *NOP(F4)             SBC(F5)              INC(F6)              ???(F7)
    {2, 2, Rel, " BEQ"}, {2, 5, InY, " SBC"}, {0, 0, Non, "****"}, {0, 0, Non, "****"}, {2, 0, ZPX, "*NOP"}, {2, 4, ZPX, " SBC"}, {2, 6, ZPX, " INC"}, {0, 0, Non, "****"},

    // SED(F8)           SBC(F9)              *NOP(FA)             ???(FB)              *NOP(FC)             SBC(FD)              INC(FE)              ???(FF)
    {1, 2, Non, " SED"}, {3, 4, AbY, " SBC"}, {1, 0, Non, "*NOP"}, {0, 0, Non, "****"}, {3, 0, AbX, "*NOP"}, {3, 4, AbX, " SBC"}, {3, 7, AbX, " INC"}, {0, 0, Non, "****"}};
}

ops::metadata ops::opcode_data(byte_t opcode)
{
    return operation_data[opcode];
}
