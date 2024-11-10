#pragma once

#include "types.h"
#include "bus.h"
#include "cpu.h"
#include "cartridge.h"
#include "ui/ppu_utils.h"

#include <array>
#include <span>
#include <cstring>


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
    // debugging
    uint64_t frame_ = 0;
    uint64_t cycle_counter_ = 0;
    
    // timings
    int16_t scanline_ = -1;
    uint16_t cycle_ = 0;

    // Nametable mirroring
    NT_Mirroring mirroring_ = NT_Mirroring::None;

    // scroll (debug)
    byte_t scroll_x_ = 0;
    byte_t scroll_y_ = 0;

    // synchronization
    bool is_in_vblank_ = false;
    bool frame_done_ = false;
    bool suppress_vblank_ = false;

    // statefull memory accesses helpers
    byte_t oamaddr_ = 0;
    byte_t read_buffer_ = 0;

    // Direct-oaM Access state
    byte_t dma_page_idx_ = 0;
    bool dma_requested_ = false;
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

    PPU_State& get_state() { return *this; }
    PPU_State const& get_state() const { return *this; }

    address_t get_vram_addr() const { return cursor_.v.get(); }

    NT_Mirroring get_mirroring() const { return mirroring_; }
    void set_mirroring(NT_Mirroring mirroring) { mirroring_ = mirroring; }

    byte_t get_foreground_half() const { return ppuctrl_.fg_pat_; }
    byte_t get_background_half() const { return ppuctrl_.bg_pat_; }

private:
    void bg_eval_();
    void fg_eval_();
    void render_();
    void tick_();

    // Output image
    Output output_;

    // memory
    std::array<byte_t, 0x1000> memory_;
    std::array<byte_t, 0x20> palette_;
    std::array<byte_t, 0x100> oam_;

    // BG rendering
    Tile bg_next_tile_;
    TileShifter bg_shifter_;

    // sprite rendering
    struct SecondaryOAM
    {
        struct Entry
        {
            byte_t lpat_ = 0;
            byte_t hpat_ = 0;
            OAMSprite::Attributes att_ = {};
            byte_t x_ = 0;

            byte_t get_pat(byte_t col)
            {
                if (col >= x_ && col < x_ + 8)
                {
                    const byte_t mask = 0x80 >> (col - x_);
                    return ((hpat_ & mask) ? 0b10 : 0b00) | ((lpat_ & mask) ? 0b01 : 0b00);
                }

                return 0;
            }
        };

        std::array<Entry, 8> list_ {};
        int count_ = 0;
        bool has_sprite_0_ = false;

        void reset();
    };
    Latch<SecondaryOAM> secondary_oam_;

    void load_sprite_(SecondaryOAM::Entry& sprite, Tile tile, OAMSprite::Attributes attrib, byte_t x, byte_t y);

    // memory access
    byte_t load_(address_t addr) const;
    void store_(address_t addr, byte_t value);

    address_t mirror_addr_(address_t addr) const;

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
            address_t _ : 1;
        };
        static_assert(sizeof(VRAM) == sizeof(address_t));

        VRAM v;
        VRAM t;

        byte_t x : 3; // Fine x scroll
        bool w : 1; // Write toggle
    } cursor_;

};