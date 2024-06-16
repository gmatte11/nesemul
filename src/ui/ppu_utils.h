#pragma once

#include "types.h"

#include <array>
#include <span>

class BUS;

namespace sf
{
    class Texture;
}

struct Color
{
    byte_t r, g, b, a = 255;
};
static_assert(sizeof(Color) == sizeof(int));

template <uint32_t W, uint32_t H>
class Image
{
public:
    static constexpr uint32_t Width = W;
    static constexpr uint32_t Height = H;

    void set(int x, int y, Color color);

    byte_t const* data() const { return buf_.data(); }
    size_t size() const { return buf_.size(); }
    size_t pitch() const { return Width * sizeof(Color); }

public:
    std::array<byte_t, sizeof(Color) * Width * Height> buf_ = {0};
};

template <uint32_t W, uint32_t H>
void Image<W, H>::set(int x, int y, Color color)
{
    size_t pos = (y * pitch()) + x * sizeof(Color);
    std::memcpy(buf_.data() + pos, &color, sizeof(Color));
}

using TileImage = Image<8, 8>;
using SpriteImage = TileImage;
using LSpriteImage = Image<8, 16>;
using NAMImage = Image<256, 240>;

class Palette
{
public:
    Palette() = default;
    Palette(std::span<const Color> colors) { std::memcpy(color_.data(), colors.data(), sizeof(color_)); }
    Palette(Color c1, Color c2, Color c3, Color c4)
    {
        color_ = {c1, c2, c3, c4};
    }

    byte_t const* raw(int index) const { return reinterpret_cast<byte_t const*>(&color_[index]); }
    Color const& get(int index) const { return color_[index]; }

    Color& operator[](int idx) { return color_[idx]; }
    const Color& operator[](int idx) const { return color_[idx]; }

private:
    std::array<Color, 4> color_;
};

constexpr Color g_palette[] = {
    /* 0x00 - 0x03 */ {84, 84, 84},    {0, 30, 116},    {8, 16, 144},    {48, 0, 136},
    /* 0x04 - 0x07 */ {68, 0, 100},    {92, 0, 48},     {84, 4, 0},      {60, 24, 0},
    /* 0x08 - 0x0B */ {32, 42, 0},     {8, 58, 0},      {0, 64, 0},      {0, 60, 0},
    /* 0x0C - 0x0F */ {0, 50, 60},     {0, 0, 0},       {0, 0, 0},       {0, 0, 0},

    /* 0x10 - 0x13 */ {152, 150, 152}, {8, 76, 196},    {48, 50, 236},   {92, 30, 228},
    /* 0x14 - 0x17 */ {136, 20, 176},  {160, 20, 100},  {152, 34, 32},   {120, 60, 0},
    /* 0x18 - 0x1B */ {84, 90, 0},     {40, 114, 0},    {8, 124, 0},     {0, 118, 40},
    /* 0x1C - 0x1F */ {0, 102, 120},   {0, 0, 0},       {0, 0, 0},       {0, 0, 0},
    
    /* 0x20 - 0x23 */ {236, 238, 236}, {76, 154, 236},  {120, 124, 236}, {176, 98, 236},
    /* 0x24 - 0x27 */ {228, 84, 236},  {236, 88, 180},  {236, 106, 100}, {212, 136, 32},
    /* 0x28 - 0x2B */ {160, 170, 0},   {116, 196, 0},   {76, 208, 32},   {56, 204, 108},
    /* 0x2C - 0x2F */ {56, 180, 204},  {60, 60, 60},    {0, 0, 0},       {0, 0, 0},

    /* 0x30 - 0x33 */ {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236},
    /* 0x34 - 0x37 */ {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
    /* 0x38 - 0x3B */ {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
    /* 0x3C - 0x3F */ {160, 214, 228}, {160, 162, 160}, {0, 0, 0},       {0, 0, 0}
};

struct Tile
{
    byte_t ntbyte_ = 0;
    byte_t atbyte_ = 0;
    byte_t half_ = 0;
    byte_t lpat_ = 0;
    byte_t hpat_ = 0;
};
static_assert(sizeof(Tile) <= sizeof(int64_t));

struct OAMSprite
{
    byte_t y_;
    byte_t tile_;
    struct Attributes : register_t<byte_t>
    {
        byte_t palette_ : 2; // index of foreground palette.
        byte_t _ : 3;
        byte_t priority_ : 1; // 0: in front, 1: behind
        byte_t h_flip_ : 1; // horizontal flip
        byte_t v_flip_ : 1; // vertical flip

    } att_;
    byte_t x_;
};
static_assert(sizeof(OAMSprite) == sizeof(int));

Palette ppu_read_palette(const BUS& bus, int idx);

namespace ui
{
    void ppu_patterntable_texture(sf::Texture& tex, std::span<const byte_t> chr_buf, const Palette& palette);
    void ppu_nametable_texture(sf::Texture& tex);
    void ppu_oam_texture(sf::Texture& tex, std::span<OAMSprite> sprites);
}