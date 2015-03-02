#include <ops.h>

namespace
{
    const ops::metadata operation_data[] =
    {
        // BRK(00) ORA(01) nop(02) nop(03) nop(04) ORA(05) ASL(06) nop(07)
           {1, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 0},

        // PHP(08) ORA(09) ASL(0A) nop(0B) nop(0C) ORA(0D) ASL(0E) nop(0F)
           {1, 3}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // nop(10) ORA(11) ASL(12) nop(13) BPL(14) ORA(15) ASL(16) nop(17)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // CLC(18) ORA(19) nop(1A) nop(1B) nop(1C) ORA(1D) ASL(1E) nop(1F)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // JSR(20) AND(21) nop(22) nop(23) BIT(24) AND(25) ROL(26) nop(27)
           {3, 6}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // PLP(28) AND(29) ROL(2A) nop(2B) BIT(2C) AND(2D) ROL(2E) nop(2F)
           {1, 4}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BMI(30) AND(31) nop(32) nop(33) nop(34) AND(35) ROL(36) nop(37)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // SEC(38) AND(39) nop(3A) nop(3B) nop(3C) AND(3D) ROL(3E) nop(3F)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // RTI(40) EOR(41) nop(42) nop(43) nop(44) EOR(45) LSR(46) nop(47)
           {1, 6}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // PHA(48) EOR(49) LSR(4A) nop(4B) JMP(4C) EOR(4D) LSR(4E) nop(4F)
           {1, 3}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BVC(50) EOR(51) nop(52) nop(53) nop(54) EOR(55) LSR(56) nop(57)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // CLI(58) EOR(59) nop(5A) nop(5B) nop(5C) EOR(5D) LSR(5E) nop(5F)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // RTS(60) ADC(61) nop(62) nop(63) nop(64) ADC(65) ROR(66) nop(67)
           {1, 6}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // PLA(68) ADC(69) ROR(6A) nop(6B) JMP(6C) ADC(6D) ROR(6E) nop(6F)
           {1, 4}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BVS(70) ADC(71) nop(72) nop(73) nop(74) ADC(75) ROR(76) nop(77)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // SEI(78) ADC(79) nop(7A) nop(7B) nop(7C) ADC(7D) ROR(7E) nop(7F)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // nop(80) STA(81) nop(82) nop(83) STY(84) STA(85) STX(86) nop(87)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // DEY(88) nop(89) TXA(8A) nop(8B) STY(8C) STA(8D) STX(8E) nop(8F)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BCC(90) STA(91) nop(92) nop(93) STY(94) STA(95) STX(96) nop(97)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // TYA(98) STA(99) TXS(9A) nop(9B) nop(9C) STA(9D) nop(9E) nop(9F)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // LDY(A0) LDA(A1) LDX(A2) nop(A3) LDY(A4) LDA(A5) LDX(A6) nop(A7)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // TAY(A8) LDA(A9) TAX(AA) nop(AB) LDY(AC) LDA(AD) LDX(AE) nop(AF)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BCS(B0) LDA(B1) nop(B2) nop(B3) LDY(B4) LDA(B5) LDX(B6) nop(B7)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // CLV(B8) LDA(B9) TSX(BA) nop(BB) LDY(BC) LDA(BD) LDX(BE) nop(BF)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // CPY(C0) CMP(C1) nop(C2) nop(C3) CPY(C4) CMP(C5) DEC(C6) nop(C7)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // INY(C8) CMP(C9) DEX(CA) nop(CB) CPY(CC) CMP(CD) DEC(CE) nop(CF)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BNE(D0) CMP(D1) nop(D2) nop(D3) nop(D4) CMP(D5) DEC(D6) nop(D7)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // CLD(D8) CMP(D9) nop(DA) nop(DB) nop(DC) CMP(DD) DEC(DE) nop(DF)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // CPX(E0) SBC(E1) nop(E2) nop(E3) CPX(E4) SBC(E5) INC(E6) nop(E7)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // INX(E8) SBC(E9) NOP(EA) nop(EB) CPX(EC) SBC(ED) INC(EE) nop(EF)
           {0, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // BEQ(F0) SBC(F1) nop(F2) nop(F3) nop(F4) SBC(F5) INC(F6) nop(F7)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},

        // SED(F8) SBC(F9) nop(FA) nop(FB) nop(FC) SBC(FD) INC(FE) nop(FF)
           {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };
}

ops::metadata ops::opcode_data(byte_t opcode)
{
    return operation_data[opcode];
}
