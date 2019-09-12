#ifndef __NESEMUL_PPU_H__
#define __NESEMUL_PPU_H__

#include "types.h"
#include "bus.h"
#include "cpu.h"
#include "cartridge.h"
#include "image.h"

#include <array>
#include <cstring>

enum class Mirroring
{
    None,
    Horizontal,
    Vertical
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
    Color const& get(int index) const { return color_[index]; }

private:
    std::array<Color, 4> color_;
};

struct Tile
{
    byte_t pixel(uint8_t x, uint8_t y, byte_t flip = 0) const;
    byte_t ntbyte_ = 0;
    byte_t atbyte_ = 0;
    byte_t half_ = 0;

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
    using Output = Image<256, 240>;

    PPU() = default;

    void init(BUS* bus, Cartridge* cart) { bus_ = bus; cart_ = cart; }

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

    inline const Output& output() const
    {
        return output_;
    }

    byte_t load(address_t addr) const { return load_(addr); }

    inline uint64_t frame() const { return frame_; }

    void set_mirroring(Mirroring mirroring) { mirroring_ = mirroring; }

    void patterntable_img(Image<128, 128>& image, byte_t half) const;
    Tile get_pattern_tile(byte_t ntbyte, byte_t half) const;

private:
    int16_t scanline_ = -1;
    uint16_t cycle_ = 0;
    uint64_t frame_ = 0;

    // Output image
    Output output_;

    // Nametable mirroring
    Mirroring mirroring_;

    // memory
    std::array<byte_t, 0x4000> memory_{};
    std::array<byte_t, 0x100> oam_{};

    // BG rendering
    std::array<Latch<Tile>, 2> bg_tiles_;

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
    Cartridge* cart_ = nullptr;

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
