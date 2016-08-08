#ifndef __NESEMUL_PPU_H__
#define __NESEMUL_PPU_H__

#include <types.h>
#include <cpu.h>
#include <array>
#include <cstring>

#include <image.h>

struct Tile;

class PPU
{
public:
    PPU(CPU & cpu, byte_t* registers, byte_t* oamdma);

    void next();
    void reset();

    byte_t* data()
    {
        return memory_.data();
    }

    const byte_t* data() const
    {
        return memory_.data();
    }

    inline const Image& output()
    {
        return output_;
    }

    void patterntable_img(byte_t* buf, int pitch, int index) const;
    Tile get_pattern_tile(address_t address) const;

    void nametable_img(byte_t *buf, int pitch, int index) const;

    void sprite_img(byte_t *buf, int pitch) const;

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

    // CPU
    CPU & cpu_;

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

    // vram cursor
    cursor_t vram_;
};

struct Tile
{
    void pixel(int index, byte_t *pixels)
    {
        // temporary RGB pallet
        static std::array<byte_t, 12> tmp_pallet = 
            {
                0x92, 0x90, 0xff, // pale blue
                0x88, 0xd8, 0x00, // green
                0x0c, 0x93, 0x00, // dark green
                0x00, 0x00, 0x00 // black
            };

        int row = index / 8;
        int col = index % 8;

        address_t laddr = (0xff00 & address_) + ((0x00ff & address_) << 4) + row;
        address_t haddr = laddr + 8;
        byte_t lpat = *(ppu_->data() + laddr);
        byte_t hpat = *(ppu_->data() + haddr);

        byte_t val = (0x1 & (lpat >> (7 - col))) | ((0x1 & (hpat >> (7 - col))) << 1);
        std::memcpy(pixels, tmp_pallet.data() + (val * 3), 3);
    }
    address_t address_;
    const PPU* ppu_;
};

#endif // __NESEMUL_PPU_H__
