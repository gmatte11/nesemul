#ifndef __NESEMUL_PPU_H__
#define __NESEMUL_PPU_H__

#include <types.h>
#include <array>

#include <image.h>

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

    inline const Image& output()
    {
        return output_;
    }

    void pattern_table(byte_t* buf, int pitch) const;

private:
    int scanline_ = -1;
    unsigned int cycle_ = 0;

    // Output image
    Image output_;

    // memory
    std::array<byte_t, 0x4000> memory_{};
    std::array<byte_t, 0xFF> oam_{};

    // shift registers
    uint16_t register_1;
    uint16_t register_2;
    uint8_t register_3;
    uint8_t register_4;

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
