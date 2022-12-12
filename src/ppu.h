#pragma once

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
    Palette() = default;
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
    byte_t ntbyte_ = 0;
    byte_t atbyte_ = 0;
    byte_t half_ = 0;
    byte_t lpat_ = 0;
    byte_t hpat_ = 0;
};

struct TileShifter
{
    address_t lpat_ = 0;
    address_t hpat_ = 0;
    address_t latt_ = 0;
    address_t hatt_ = 0;

    void load(Tile const& next_tile)
    {
        lpat_ = (lpat_ & 0xFF00) | next_tile.lpat_;
        hpat_ = (hpat_ & 0xFF00) | next_tile.hpat_;

        latt_ = (latt_ & 0xFF00) | ((next_tile.atbyte_ & 0b01) ? 0xFF : 0x00);
        hatt_ = (hatt_ & 0xFF00) | ((next_tile.atbyte_ & 0b10) ? 0xFF : 0x00);
    }

    void shift()
    {
        lpat_ <<= 1;
        hpat_ <<= 1;
        latt_ <<= 1;
        hatt_ <<= 1;
    }
};

struct Sprite
{
    byte_t lpat_ = 0;
    byte_t hpat_ = 0;
    byte_t att_ = 0;
    int16_t x_ = 0;

    bool is_visible() const
    {
        return x_ <= 0 && x_ > -8;
    }

    void shift()
    {
        --x_;
        if (x_ < 0)
        {
            lpat_ <<= 1;
            hpat_ <<= 1;
        }
    }
};


template <typename T>
struct Latch
{
    T& store() { return (selector) ? states_[0] : states_[1]; }
    T& read() { return (!selector) ? states_[0] : states_[1]; }
    void flip() { selector = !selector; }

    std::array<T, 2> states_;
    bool selector = false;
};

struct PPU_State
{
    // timings
    uint64_t frame_ = 0;
    int16_t scanline_ = -1;
    uint16_t cycle_ = 0;

    // Nametable mirroring
    Mirroring mirroring_;

    // scroll (debug)
    byte_t scroll_x_;
    byte_t scroll_y_;
    bool is_in_vblank_ = false;
    bool frame_done_ = false;
};

class PPU : private PPU_State
{
public:
    using Output = Image<256, 240>;

    PPU() = default;

    void init(BUS* bus)
    {
        bus_ = bus;
    }

    void step();
    void reset();

    bool on_write_cpu(address_t addr, byte_t value);
    bool on_read_cpu(address_t addr, byte_t& value);

    bool on_write_ppu(address_t addr, byte_t value);
    bool on_read_ppu(address_t addr, byte_t& value);

    void dma_copy_byte(byte_t rw_cycle);

    byte_t* data()
    {
        return memory_.data();
    }

    const byte_t* data() const
    {
        return memory_.data();
    }

    byte_t* oam_data()
    {
        return oam_.data();
    }

    const byte_t* oam_data() const
    {
        return oam_.data();
    }

    inline const Output& output() const
    {
        return output_;
    }

    byte_t load(address_t addr) const { return load_(addr); }

    inline uint64_t frame() const { return frame_; }
    
    bool grab_dma_request()
    {
        bool requested = dma_requested_;
        dma_requested_ = false;
        return requested;
    }

    bool grab_frame_done()
    {
        bool done = frame_done_;
        frame_done_ = false;
        return done;
    } 

    bool is_at_end_of_line()
    {
        return cycle_ == 320;
    }

    PPU_State& get_state() { return *this; }
    PPU_State const& get_state() const { return *this; }

    void set_mirroring(Mirroring mirroring) { mirroring_ = mirroring; }

    void patterntable_img(Image<128, 128>& image, byte_t half, Palette const& palette) const;
    void tile_img(Image<8, 8>& image, byte_t ntbyte, byte_t half, Palette const& palette) const;
    Tile get_pattern_tile(byte_t ntbyte, byte_t half) const;

    void nametable_img(Output& image, byte_t nam_idx) const;

    Palette get_palette(byte_t idx) const;

private:
    void bg_eval_();
    void fg_eval_();
    void render_();
    void tick_();

    // Output image
    Output output_;

    // memory
    std::array<byte_t, 0x4000> memory_ {};
    std::array<byte_t, 0x100> oam_ {};

    // BG rendering
    Tile bg_next_tile_;
    TileShifter bg_shifter_;

    // sprite rendering
    struct SecondaryOAM
    {
        struct Entry
        {
            Sprite sprite_;
            int oam_idx_;
        };
        std::array<Entry, 8> list_ {};
        int count_ = 0;

        static const Entry empty_;
    };
    Latch<SecondaryOAM> secondary_oam_;

    void load_sprite_(Sprite& sprite, Tile const& tile, byte_t attrib, byte_t x, byte_t y);

    // memory access
    byte_t load_(address_t addr) const;
    void store_(address_t addr, byte_t value);

    address_t mirror_addr_(address_t addr) const;

    // attribute tables access (palettes)
    byte_t get_attribute_(address_t ntaddr, int row, int col) const;

    // Compute pixel from tile
    byte_t get_pixel(Tile const& tile, uint8_t x, uint8_t y, byte_t flip = 0) const;

    // Connected devices
    BUS* bus_ = nullptr;

    // registers
    struct : public register_t<byte_t>
    {
        byte_t nam_x_ : 1; // (bit 0)
        byte_t nam_y_ : 1; // (bit 1)
        byte_t addr_inc_ : 1; // (bit 2)
        byte_t fg_pat_ : 1; // (bit 3)
        byte_t bg_pat_ : 1; // (bit 4)
        byte_t sprite_size_ : 1; // (bit 5)
        byte_t master_ : 1; // (bit 6)
        byte_t nmi_ : 1; // (bit 7)
    } ppuctrl_;
    static_assert(sizeof(decltype(ppuctrl_)) == sizeof(byte_t));

    struct : public register_t<byte_t>
    {
        byte_t greyscale_ : 1; // (bit 0)
        byte_t left_bg_ : 1; // (bit 1)
        byte_t left_fg_ : 1; // (bit 2)
        byte_t render_bg_ : 1; // (bit 3)
        byte_t render_fg_ : 1; // (bit 4)
        byte_t emp_red_ : 1; // (bit 5)
        byte_t emp_green_ : 1; // (bit 6)
        byte_t emp_blue_ : 1; // (bit 7)
    } ppumask_;
    static_assert(sizeof(decltype(ppumask_)) == sizeof(byte_t));

    struct : public register_t<byte_t>
    {
        byte_t garbage_ : 5; // (bits 0-4)
        byte_t sprite_overflow_ : 1; // (bit 5)
        byte_t sprite_0_hit_ : 1; // (bit 6)
        byte_t vblank_ : 1; // (bit 7)
    } ppustatus_;
    static_assert(sizeof(decltype(ppustatus_)) == sizeof(byte_t));

    byte_t oamaddr_;

    // vram cursor (PPUSCROLL, PPUADDR and PPUDATA)
    struct Cursor
    {
        struct VRAM : public register_t<address_t>
        {
            address_t X : 5; // Coarse X scroll (bits 0-4)
            address_t Y : 5; // Coarse Y scroll (bits 5-9)
            address_t NX : 1; // Nametable X component (bit 10)
            address_t NY : 1; // Nametable Y component (bit 11)
            address_t y : 3; // Fine y scroll (bits 12-14)
        };
        static_assert(sizeof(VRAM) == sizeof(address_t));

        VRAM v;
        VRAM t;

        byte_t x : 3; // Fine x scroll
        bool w : 1; // Write toggle
    } cursor_;
    byte_t read_buffer_ = 0;

    byte_t dma_page_idx_ = 0;
    bool dma_requested_ = false;

    friend class PageDebugPPU;
};