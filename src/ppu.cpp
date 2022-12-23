#include "ppu.h"

#include "debugger.h"
#include "ram.h"

#include <cstring>

static Color g_palette[] = {
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

const PPU::SecondaryOAM::Entry PPU::SecondaryOAM::empty_{Sprite{0xFF, 0xFF, 0xFF, 0xFF}, 0xFF};

void PPU::step()
{
    bg_eval_();
    fg_eval_();
    render_();
    tick_();
}

void PPU::reset()
{
    scanline_ = -1;
    cycle_ = 0;
    frame_ = 0;
    cycle_counter_ = 0;

    ppuctrl_.set(0);
    ppumask_.set(0);

    scroll_x_ = 0;
    scroll_y_ = 0;
    is_in_vblank_ = false;

    cursor_.v.set(0);
    cursor_.t.set(0);
    cursor_.x = 0;
    cursor_.w = false;
    read_buffer_ = 0;

    dma_requested_ = false;

    oam_.fill(0xFF);
}

bool PPU::on_write_cpu(address_t addr, byte_t value)
{
    // Mirroring
    if (addr >= 0x2008 && addr < 0x4000)
        addr = 0x2000 + (addr & 0x7);

    switch (addr)
    {
        // ppuctrl
    case 0x2000:
        {
        const bool was_nmi_enabled = ppuctrl_.nmi_;
        ppuctrl_.set(value);

        if (ppustatus_.vblank_ && !was_nmi_enabled && ppuctrl_.nmi_)
            bus_->cpu_.pull_nmi();

        cursor_.t.NX = ppuctrl_.nam_x_;
        cursor_.t.NY = ppuctrl_.nam_y_;
        }
        return true;

        // ppumask
    case 0x2001:
        ppumask_.set(value);
        return true;

        // oamaddr
    case 0x2003:
        oamaddr_ = value;
        return true;

        //oamdata
    case 0x2004:
        oam_[oamaddr_] = value;
        ++oamaddr_;
        return true;

        // ppuscroll
    case 0x2005:
    {
        if (!cursor_.w)
        {
            scroll_x_ = value;
            cursor_.t.X = (value >> 3) & 0x1F;
            cursor_.x = value & 0x07;
        }
        else
        {
            scroll_y_ = value;
            cursor_.t.Y = (value >> 3) & 0x1F;
            cursor_.t.y = value & 0x07;
        }

        cursor_.w = !cursor_.w;
        return true;
    }

        // ppuaddr
    case 0x2006:
    {
        if (!cursor_.w)
        {
            cursor_.t.set_h(value & 0x3F);
        }
        else
        {
            cursor_.t.set_l(value);
            cursor_.v = cursor_.t;
        }

        cursor_.w = !cursor_.w;
        return true;
    }

        // ppudata
    case 0x2007:
        store_(cursor_.v.get() & 0x3FFF, value);
        cursor_.v += (ppuctrl_.addr_inc_) ? 32 : 1;
        cursor_.v &= 0x7FFF;
        return true;

        // oamdma
    case 0x4014:
    {
        //NES_ASSERT(oamaddr_ == 0);

        dma_requested_ = true;
        dma_page_idx_ = value;

        return true;
    }
    }

    return false;
}

bool PPU::on_read_cpu(address_t addr, byte_t& value)
{
    // Mirroring
    if (addr >= 0x2008 && addr < 0x4000)
        addr = 0x2000 + (addr & 0x7);

    switch (addr)
    {
    case 0x2002:
        value = ppustatus_.get();
        ppustatus_.vblank_ = 0;
        cursor_.w = false;

        if (scanline_ == 241 && cycle_ <=  1)
            suppress_vblank_ = true;

        return true;

    case 0x2004:
        value = oam_[oamaddr_];
        return true;

    case 0x2007:
    {
        const address_t vram_addr = cursor_.v.get() & 0x3FFF;

        value = read_buffer_;
        read_buffer_ = load_(vram_addr);

        // palette read is instantaneous
        if (vram_addr >= 0x3F00 && vram_addr < 0x4000)
            value = read_buffer_;

        cursor_.v += (ppuctrl_.addr_inc_) ? 32 : 1;
        cursor_.v &= 0x7FFF;

        return true;
    }
    }

    return false;
}

bool PPU::on_write_ppu(address_t addr, byte_t value)
{
    addr = mirror_addr_(addr);
    
    if (addr >= 0x2000 && addr < 0x3000)
    {
        memory_[addr - 0x2000] = value;
        return true;
    }

    if (addr >= 0x3F00 && addr < 0x3F20)
    {
        palette_[addr & 0xFF] = value;
        return true;
    }

    return false;
}

bool PPU::on_read_ppu(address_t addr, byte_t& value)
{
    addr = mirror_addr_(addr);
    
    if (addr >= 0x2000 && addr < 0x3000)
    {
        value = memory_[addr - 0x2000];
        return true;
    }

    if (addr >= 0x3F00 && addr < 0x3F20)
    {
        value = palette_[addr & 0xFF];
        return true;
    }

    return false;
}

void PPU::dma_copy_byte(byte_t rw_cycle)
{
    NES_ASSERT(rw_cycle >= 0 && rw_cycle < 256);
    address_t page_addr = dma_page_idx_ << 8;

    byte_t data = bus_->read_cpu(page_addr | rw_cycle);
    oam_[rw_cycle] = data;
}

void PPU::patterntable_img(Image<128, 128>& image, byte_t index, Palette const& palette) const
{
    for (uint8_t y = 0; y < 128; ++y)
    {
        for (uint8_t x = 0; x < 128; ++x)
        {
            byte_t tx = x / 8;
            byte_t ty = y / 8;
            byte_t ntbyte = (ty << 4 | tx);

            Tile tile = get_pattern_tile(ntbyte, index);
            byte_t pixel = get_pixel(tile, x % 8, y % 8);
            image.set(x, y, palette.get(pixel));
        }
    }
}

void PPU::tile_img(Image<8, 8>& image, byte_t ntbyte, byte_t half, Palette const& palette) const
{
    Tile tile = get_pattern_tile(ntbyte, half);

    for (uint8_t y = 0; y < 8; ++y)
    {
        for (uint8_t x = 0; x < 8; ++x)
        {
            byte_t pixel = get_pixel(tile, x, y);
            image.set(x, y, palette.get(pixel));
        }
    }
}

Tile PPU::get_pattern_tile(byte_t ntbyte, byte_t half) const
{
    Tile t;
    t.ntbyte_ = ntbyte;
    t.half_ = half & 0x1;
    return t;
}

void PPU::nametable_img(Output& image, byte_t nam_idx) const
{
    static const address_t bg_palette_addr[] = {0x3F01, 0x3F05, 0x3F09, 0x3F0D};
    static const address_t nametable_addr[] = {0x2000, 0x2400, 0x2800, 0x2C00};

    address_t ntaddr = nametable_addr[nam_idx];
    Tile tile;

    for (int ntrow = 0; ntrow < (240 / 8); ++ntrow)
    {
        for (int ntcol = 0; ntcol < (256 / 8); ++ntcol)
        {
            tile.ntbyte_ = load_(ntaddr | static_cast<address_t>(ntrow * 32 + ntcol));
            tile.atbyte_ = get_attribute_(ntaddr, ntrow / 2, ntcol / 2);
            tile.half_ = ppuctrl_.bg_pat_;

            address_t paladdr = bg_palette_addr[tile.atbyte_];
            Palette palette(
                g_palette[load_(0x3F00)],
                g_palette[load_(paladdr + 0)],
                g_palette[load_(paladdr + 1)],
                g_palette[load_(paladdr + 2)]);

            for (uint8_t trow = 0; trow < 8; ++trow)
            {
                for (uint8_t tcol = 0; tcol < 8; ++tcol)
                {
                    int col = (ntcol * 8) + tcol;
                    int row = (ntrow * 8) + trow;

                    byte_t pixel = get_pixel(tile, tcol, trow);
                    image.set(col, row, palette.get(pixel));
                }
            }
        }
    }
}

Palette PPU::get_palette(byte_t idx) const
{
    if (idx < 4)
    {
        static constexpr address_t palette_bg[] = {0x3F01, 0x3F05, 0x3F09, 0x3F0D};
        address_t paladdr = palette_bg[idx];
        return Palette(
            g_palette[load_(0x3F00)],
            g_palette[load_(paladdr + 0)],
            g_palette[load_(paladdr + 1)],
            g_palette[load_(paladdr + 2)]
        );
    }
    else
    {
        static constexpr address_t palette_fg[] = {0x3F11, 0x3F15, 0x3F19, 0x3F1D};
        address_t paladdr = palette_fg[idx - 4];
        return Palette(
            {0xFF, 0xFF, 0xFF, 0x00},
            g_palette[load_(paladdr + 0)],
            g_palette[load_(paladdr + 1)],
            g_palette[load_(paladdr + 2)]
        );
    }
}

void PPU::bg_eval_()
{
    auto& v = cursor_.v;

    if (ppumask_.render_bg_ || ppumask_.render_fg_)
    {
        if (cycle_ == 256)
        {
            // Vertical increment
            if (v.y < 7)
            {
                v.y++;
            }
            else
            {
                v.y = 0;
                if (v.Y == 29)
                {
                    v.Y = 0;
                    v.NY = ~v.NY;
                }
                else if (v.Y == 31)
                {
                    v.Y = 0;
                }
                else
                {
                    v.Y++;
                }
            }
        }

        // Transfer horizontal scroll bits
        if (cycle_ == 257)
        {
            v.X = cursor_.t.X;
            v.NX = cursor_.t.NX;
        }

        if (scanline_ >= -1 && scanline_ < 240)
        {
            if ((cycle_ > 0 && cycle_ <= 256) || (cycle_ > 320 && cycle_ <= 336))
            {
                Tile& tile = bg_next_tile_;

                int op = (cycle_ - 1) % 8;
                switch (op)
                {
                case 0:
                    bg_shifter_.load(tile);
                    break;

                case 1: // NT byte
                {
                    address_t ntaddr = 0x2000 | (v.get() & 0x0FFF);
                    tile.ntbyte_ = load_(ntaddr);
                }
                break;

                case 3: // AT byte
                {
                    address_t ataddr = 0x23C0 | (v.get() & 0x0C00) | ((v.get() >> 4) & 0x38) | ((v.get() >> 2) & 0x07);
                    byte_t areashift = ((v.Y & 0x02) ? 4 : 0) + ((v.X & 0x02) ? 2 : 0);
                    tile.atbyte_ = (load_(ataddr) >> areashift) & 0x3;
                }
                break;

                case 5: // Low PAT byte
                {
                    tile.half_ = ppuctrl_.bg_pat_ & 0x1;

                    address_t addr
                        = static_cast<address_t>(tile.half_) << 12
                        | static_cast<address_t>(tile.ntbyte_) << 4
                        | static_cast<address_t>(v.y);

                    tile.lpat_ = load_(addr);
                }
                break;

                case 7: // High PAT byte
                {
                    address_t addr
                        = static_cast<address_t>(tile.half_) << 12
                        | static_cast<address_t>(tile.ntbyte_) << 4
                        | static_cast<address_t>(v.y);

                    tile.hpat_ = load_(addr + 8);
                }

                // Increment horizontal scroll bits
                {
                    if (v.X == 31)
                    {
                        v.X = 0;
                        v.NX = ~v.NX;
                    }
                    else
                    {
                        v.X++;
                    }
                }
                break;
                }
            }
        }
    }
}

void PPU::fg_eval_()
{
    if (!ppumask_.render_fg_ && !ppumask_.render_bg_)
        return;

    if (scanline_ >= 0 && scanline_ < 240)
    {
        if (cycle_ == 1)
        {
            secondary_oam_.flip();
            secondary_oam_.store().count_ = 0;
            secondary_oam_.store().list_.fill(SecondaryOAM::empty_);
        }

        if (cycle_ == 256)
        {
            for (int i = 0; i < 64; ++i)
            {
                struct Entry
                {
                    byte_t y;
                    byte_t tile;
                    byte_t att;
                    byte_t x;
                };

                Entry& entry = reinterpret_cast<Entry*>(oam_.data())[i];
                int y = scanline_ + 1;

                if (entry.y < y && (y - entry.y) <= 8)
                {
                    auto& sprites = secondary_oam_.store();
                    if (sprites.count_ < 8)
                    {
                        auto& sprite = sprites.list_[sprites.count_];
                        sprite.oam_idx_ = i;

                        Tile tile = get_pattern_tile(entry.tile, ppuctrl_.fg_pat_);
                        load_sprite_(sprite.sprite_, tile, entry.att, entry.x + 1, static_cast<uint8_t>(y) - entry.y - 1);

                        ++sprites.count_;
                    }
                    else
                    {
                        // TODO? Sprite overflow bug
                        ppustatus_.sprite_overflow_ = 1;
                    }
                }
            }
        }

        if (cycle_ > 256 && cycle_ <= 320)
        {
            oamaddr_ = 0;
        }
    }
}

void PPU::render_()
{
    if (scanline_ >= 0 && scanline_ < 240)
    {
        if (cycle_ > 0 && cycle_ <= 256)
        {
            int row = scanline_;
            int col = cycle_ - 1;
            
            byte_t bg_pixel = 0;
            byte_t fg_pixel = 0;
            byte_t bg_pal = 0;
            byte_t fg_pal = 0;
            bool fg_priority = false;
            bool has_sprite_0 = false;

            if (ppumask_.render_bg_)
            {
                address_t mux = 0x8000 >> cursor_.x;

                bg_pixel = ((bg_shifter_.hpat_ & mux) ? 0b10 : 0b00) | ((bg_shifter_.lpat_ & mux) ? 0b01 : 0b00);
                bg_pal = ((bg_shifter_.hatt_ & mux) ? 0b10 : 0b00) | ((bg_shifter_.latt_ & mux) ? 0b01 : 0b00);
            }

            bg_shifter_.shift();

            {
                auto& sprites = secondary_oam_.read();

                for (int i = 0; i < sprites.count_; ++i)
                {
                    Sprite& sprite = sprites.list_[i].sprite_;
                    sprite.shift();

                    if (fg_pixel == 0 && sprite.is_visible() && ppumask_.render_fg_) // TODO 8x16 sprites
                    {
                        fg_pixel = ((sprite.hpat_ & 0x80) ? 0b10 : 0b00) | ((sprite.lpat_ & 0x80) ? 0b01 : 0b00);
                        fg_pal = sprite.att_ & 0x3;
                        fg_priority = (sprite.att_ & 0x20) == 0;
                        
                        has_sprite_0 = has_sprite_0 || sprites.list_[i].oam_idx_ == 0;
                    }
                }
            }

            const bool render_fg = ppumask_.render_fg_ && fg_pixel != 0 && (bg_pixel == 0 || fg_priority);
            const bool render_bg = ppumask_.render_bg_;
            const bool sprite_0_eval = has_sprite_0 && ppumask_.render_fg_ && col < 255;

            if (sprite_0_eval)
            {
                if (fg_pixel != 0 && bg_pixel != 0)
                    ppustatus_.sprite_0_hit_ = 1;
            }

            Color color{0, 0, 0, 0xFF};

            if (render_fg)
            {
                color = get_palette(fg_pal + 4).get(fg_pixel);
            }
            else if (render_bg)
            {
                color = get_palette(bg_pal).get(bg_pixel);
            }

            output_.set(col, row, color);
        }
    }
}

void PPU::tick_()
{
    // scanline -1: dummy scanline, single cycle skipped on odd-frame, vblank is unset.
    // scanlines 0..239: rendering scanlines
    // scanline  240: empty scanline.
    // scanlines 241-260: vblank is set a the 2nd cycle of the 241st scanline and also trigger the vblank's NMI.

    const bool rendering_enabled = ppumask_.render_bg_ || ppumask_.render_fg_;
    
    if (scanline_ == -1)
    {
        // cycle skip on odd frame
        if (rendering_enabled && cycle_ == 339 && (frame_ & 0x1) != 0)
            ++cycle_;
    }

    ++cycle_counter_;

    ++cycle_;
    if (cycle_ > 340)
    {
        cycle_ = 0;
        ++scanline_;

        Debugger::instance()->on_ppu_line();

        if (scanline_ > 260)
        {
            scanline_ = -1;
            ++frame_;
            frame_done_ = true;
            Debugger::instance()->on_ppu_frame();
        }
    }

    // dummy (pre-render) scanline
    if (scanline_ == -1)
    {
        if (cycle_ == 1)
        {
            ppustatus_.vblank_ = 0; // end of vblank
            is_in_vblank_ = false;
            ppustatus_.sprite_0_hit_ = 0;
            ppustatus_.sprite_overflow_ = 0;
        }

        // Transfer vertical scroll bits
        if (rendering_enabled && cycle_ >= 280 && cycle_ <= 304)
        {
            cursor_.v.Y = cursor_.t.Y;
            cursor_.v.y = cursor_.t.y;
            cursor_.v.NY = cursor_.t.NY;
        }
    }

    // first scanline of vblank
    if (scanline_ == 241 && cycle_ == 1)
    {
        if (!suppress_vblank_)
            ppustatus_.vblank_ = 1; // start of vblank
        suppress_vblank_ = false;
        is_in_vblank_ = true;
    }

    if (scanline_ == 241 && cycle_ == 20)
    {
        if (ppustatus_.vblank_ && ppuctrl_.nmi_)
            bus_->cpu_.pull_nmi();
    }
}

void PPU::load_sprite_(Sprite& sprite, Tile const& tile, byte_t attrib, byte_t x, byte_t ty)
{
    sprite.hpat_ = sprite.lpat_ = 0;
    sprite.att_ = attrib;
    sprite.x_ = x;

    byte_t flip = attrib >> 6;
    if (0b10 & flip) ty = 7 - ty; //vert flip

    address_t addr = static_cast<address_t>(tile.half_ & 0x1) << 12
        | static_cast<address_t>(tile.ntbyte_) << 4
        | static_cast<address_t>(ty & 0b111) << 0;
 
    byte_t lpat = load_(addr);
    byte_t hpat = load_(addr + 8);

    for (byte_t tx = 0; tx < 8; ++tx)
    {
        byte_t local_x = tx;
        if (0b01 & flip) local_x = 7 - tx; //hori flip
        sprite.lpat_ |= ((lpat >> local_x) & 0x1) << tx;
        sprite.hpat_ |= ((hpat >> local_x) & 0x1) << tx;
    }
}

byte_t PPU::load_(address_t addr) const
{
    return bus_->read_ppu(addr);
}

void PPU::store_(address_t addr, byte_t value)
{
    bus_->write_ppu(addr, value);
}

address_t PPU::mirror_addr_(address_t addr) const
{
    NES_ASSERT(addr < 0x4000);

    // $3F20-$3FFF mirrors $3F00-$3F1F
    if (addr >= 0x3F20 && addr < 0x4000)
        addr &= 0x3F1F;

    // Palette mirrors
    if (addr == 0x3F10 || addr == 0x3F14 || addr == 0x3F18 || addr == 0x3F1C)
        addr -= 0x10;

    // $3000-$3EFF mirrors $2000-$2EFF
    if (addr >= 0x3000 && addr < 0x3F00)
        addr -= 0x1000;

    switch (mirroring_)
    {
    case NT_Mirroring::Vertical:
        if (addr >= 0x2800 && addr < 0x3000)
            addr -= 0x0800;
        break;

    case NT_Mirroring::Horizontal:
        if (addr >= 0x2400 && addr < 0x2800)
            addr -= 0x0400;
        
        if (addr >= 0x2C00 && addr < 0x3000)
            addr -= 0x0400;
        break;

    default:
        break;
    }

    return addr;
}

byte_t PPU::get_attribute_(address_t ntaddr, int row, int col) const
{
    // The attribute table is located at the end of the nametable (offset $03C0)
    address_t ataddr = ntaddr + 0x03C0;

    // Offset of the requested byte
    ataddr += static_cast<address_t>((row / 2) * 0x8 + (col / 2));

    byte_t metatile = load_(ataddr);

    int quadrant = 
        ((row % 2 == 1) ? 0b10 : 0) | 
        ((col % 2 == 1) ? 0b01 : 0);
        
    return 0b11 & (metatile >> (quadrant * 2));
}

byte_t PPU::get_pixel(Tile const& tile, uint8_t x, uint8_t y, byte_t flip /*= 0*/) const
{
    if (0b01 & flip) x = 7 - x; //hori flip
    if (0b10 & flip) y = 7 - y; //vert flip

    address_t addr = 
        static_cast<address_t>(tile.half_ & 0x1) << 12 |
        static_cast<address_t>(tile.ntbyte_) << 4 |
        static_cast<address_t>(y & 0b111) << 0;

    byte_t lpat = load_(addr);
    byte_t hpat = load_(addr + 8);

    byte_t mask = 1 << (7 - x);
    byte_t pixel = 
        (((hpat & mask) != 0) ? 0b10 : 0) | 
        (((lpat & mask) != 0) ? 0b01 : 0);

    return pixel;
}