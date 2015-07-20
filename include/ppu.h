#ifndef __NESEMUL_PPU_H__
#define __NESEMUL_PPU_H__

#include <types.h>
#include <array>

class PPU
{
public:
    PPU(byte_t* registers, byte_t* oamdma);

    void next();
    void reset();

    byte_t* data()
    {
        return memory_.data();
    }

private:
    int scanline_ = -1;
    unsigned int cycle_ = 0;

    // memory
    std::array<byte_t, 0x4000> memory_{};
    std::array<byte_t, 0xFF> oam_{};

    // registers mapped to CPU's memory
    byte_t& ppuctrl_;
    byte_t& ppumask_;
    byte_t& ppustatus_;
    byte_t& oamaddr_;
    byte_t& oamdata_;
    byte_t& ppuscroll_;
    byte_t& ppuaddr_;
    byte_t& ppudata_;
    byte_t& oamdma_;
};

#endif // __NESEMUL_PPU_H__
