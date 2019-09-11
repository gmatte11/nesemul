#ifndef __NESEMUL_PPU_H__
#define __NESEMUL_PPU_H__

#include "types.h"
#include "bus.h"
#include "cpu.h"
#include "image.h"

#include <array>
#include <cstring>

enum class Mirroring
{
    None,
    Horizontal,
    Vertical
};

struct Color
{
    byte_t r, g, b, a = 255;
};

class Palette
{
public:
    Palette(Color *first) { std::memcpy(color_.data(), first, sizeof(color_)); }
    Palette(Color const& c1, Color const& c2, Color const& c3, Color const& c4)
    {
        color_ = {c1, c2, c3, c4};
    }

    byte_t const* raw(int index) const { return reinterpret_cast<byte_t const*>(&color_[index]); }

private:
    std::array<Color, 4> color_;
};

struct Tile
{
    void pixel(int index, byte_t *pixels, const Palette & palette);
    address_t address_;
    const PPU* ppu_;
};

struct Sprite
{
    byte_t y_;
    byte_t tile_;
    byte_t att_;
    byte_t x_;
};

template <typename T>
struct Latch
{
    T& store() { return (selector) ? states_[0] : states_[1]; }
    T const& read() const { return (!selector) ? states_[0] : states_[1]; }
    void flip() { selector = !selector; }

    std::array<T, 2> states_;
    bool selector = false;
};

class PPU
{
public:
    PPU() = default;

    void init(BUS* bus) { bus_ = bus; }

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

    inline int frame() const { return frame_; }

    void set_mirroring(Mirroring mirroring) { mirroring_ = mirroring; }

    void patterntable_img(byte_t* buf, int pitch, int index) const;
    Tile get_pattern_tile(address_t address) const;

    void nametable_img(byte_t *buf, int pitch, int index) const;

    void sprite_img(byte_t *buf, int pitch) const;

private:
    unsigned int scanline_ = 261;
    unsigned int cycle_ = 0;
    unsigned int frame_ = 0;

    // Output image
    Image output_;

    // Nametable mirroring
    Mirroring mirroring_;

    // memory
    std::array<byte_t, 0x4000> memory_{};
    std::array<byte_t, 0x100> oam_{};

    // BG rendering
    Latch<Tile> bg_tile_1_;
    Latch<Tile> bg_tile_2_;

    // sprite rendering
    struct SecondaryOAM {
        std::array<Sprite, 8> list_{};
        int count_ = 0;
    };
    Latch<SecondaryOAM> secondary_oam_;

    // memory access
    byte_t load_(address_t addr) const;
    void store_(address_t addr, byte_t value);

    address_t mirror_addr_(address_t addr) const;

    // attribute tables access (palettes)
    byte_t get_attribute_(address_t ntaddr, int row, int col) const;

    // Connected devices
    BUS* bus_ = nullptr;

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
#endif // __NESEMUL_PPU_H__
