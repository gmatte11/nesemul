#ifndef __NESEMUL_PPU_H__
#define __NESEMUL_PPU_H__

#include <types.h>
#include <bus.h>
#include <cpu.h>
#include <array>
#include <cstring>

#include <image.h>

enum class Mirroring
{
    None,
    Horizontal,
    Vertical
};

struct Tile;

class PPU
{
public:
    PPU(BUS& bus);

    void next();
    void reset();

    bool on_write(address_t addr, byte_t value);
    bool on_read(address_t addr, byte_t& value);

    byte_t* data()
    {
        return memory_.data();
    }

    const byte_t* data() const
    {
        return memory_.data();
    }

    inline const Image& output() const
    {
        return output_;
    }

    void set_mirroring(Mirroring mirroring) { mirroring_ = mirroring; }

    void patterntable_img(byte_t* buf, int pitch, int index) const;
    Tile get_pattern_tile(address_t address) const;

    void nametable_img(byte_t *buf, int pitch, int index) const;

    void sprite_img(byte_t *buf, int pitch) const;

private:
    int scanline_ = -1;
    unsigned int cycle_ = 0;

    // Output image
    Image output_;

    // Nametable mirroring
    Mirroring mirroring_;

    // memory
    std::array<byte_t, 0x4000> memory_{};
    std::array<byte_t, 0xFF> oam_{};

    // memory access
    byte_t load_(address_t addr) const;
    void store_(address_t addr, byte_t value);

    address_t mirror_addr_(address_t addr) const;

    // attribute tables access (palettes)
    byte_t get_attribute_(address_t ntaddr, int row, int col) const;

    // shift registers
    //uint16_t register_1;
    //uint16_t register_2;
    //uint8_t register_3;
    //uint8_t register_4;

    // Connected devices
    BUS& bus_;

    // registers
    byte_t ppuctrl_;
    byte_t ppumask_;
    byte_t ppustatus_;
    byte_t oamaddr_;
    byte_t oamdata_;
    byte_t ppuscroll_;
    byte_t ppuaddr_;
    byte_t ppudata_;
    //byte_t oamdma_;

    // vram cursor
    cursor_t vram_;
    bool vram_hilo_ = false; //switch between reading hi or low byte from vram
};

struct Color
{
    byte_t r, g, b;
};

class Palette
{
public:
    Palette(Color *first) : color_(first) {}

    byte_t* raw(int index) const { return reinterpret_cast<byte_t*>(color_ + index); }

private:
    Color* color_;
};

struct Tile
{
    void pixel(int index, byte_t *pixels, const Palette & palette)
    {
        int row = index / 8;
        int col = index % 8;

        address_t laddr = (0xff00 & address_) + ((0x00ff & address_) << 4) + row;
        address_t haddr = laddr + 8;
        byte_t lpat = *(ppu_->data() + laddr);
        byte_t hpat = *(ppu_->data() + haddr);

        byte_t val = (0x1 & (lpat >> (7 - col))) | ((0x1 & (hpat >> (7 - col))) << 1);
        std::memcpy(pixels, palette.raw(val), 3);
    }
    address_t address_;
    const PPU* ppu_;
};

#endif // __NESEMUL_PPU_H__
