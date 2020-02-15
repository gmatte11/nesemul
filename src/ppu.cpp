#include "ppu.h"
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


void PPU::step()
{
    tick_();
    render_();
    bg_eval_();
    fg_eval_();
}

void PPU::reset()
{
    scanline_ = -1;
    cycle_ = 0;
    frame_ = 0;

    ppuctrl_.byte_ = 0;
    ppumask_.byte_ = 0;

    scroll_x_ = 0;
    scroll_y_ = 0;
    cursor_.v.addr = 0;
    cursor_.t.addr = 0;
    cursor_.x = 0;
    cursor_.w = false;
    read_buffer_ = 0;

    oam_.fill(0xFF);
}

bool PPU::on_write(address_t addr, byte_t value)
{
    // Mirroring
    if (addr >= 0x2008 && addr < 0x4000)
        addr = 0x2000 + (addr & 0x7);

    switch (addr)
    {
    case 0x2000:
        ppuctrl_.byte_ = value;
        cursor_.t.N = ppuctrl_.nam_;
        return true;

    case 0x2001:
        ppumask_.byte_ = value;
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
            cursor_.t.X = value >> 3;
            cursor_.x = value & 0x07;
        }
        else
        {
            scroll_y_ = value;
            cursor_.t.Y = value >> 3;
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
            cursor_.t.h = value & 0x3F;
        }
        else
        {
            cursor_.t.l = value;
            cursor_.v.addr = cursor_.t.addr;
        }

        cursor_.w = !cursor_.w;
        return true;
    }

        // ppudata
    case 0x2007:
    {
        store_(cursor_.v.addr, value);
        cursor_.v.addr += (ppuctrl_.addr_inc_) ? 32 : 1;
        cursor_.v.addr &= 0x3FFF;
        return true;
    }

        // oamdma
    case 0x4014:
    {
        address_t page_addr = value << 8;
        // TODO allow copy from cartige RAM or ROM
        bus_->ram_.memcpy(oam_.data(), page_addr, 0xFF * sizeof(byte_t));

        // TODO timing
        return true;
    }
    }

    return false;
}

bool PPU::on_read(address_t addr, byte_t& value)
{
    // Mirroring
    if (addr >= 0x2008 && addr < 0x4000)
        addr = 0x2000 + (addr & 0x7);

    switch (addr)
    {
    case 0x2002:
        value = ppustatus_.byte_;
        ppustatus_.vblank_ = 0;
        cursor_.w = false;
        return true;

    case 0x2004:
        value = oam_[oamaddr_];
        return true;

    case 0x2007:
        value = read_buffer_;
        read_buffer_ = load_(cursor_.v.addr);

        if (cursor_.v.addr >= 0x3F00)
            value = read_buffer_;

        cursor_.v.addr += (ppuctrl_.addr_inc_) ? 32 : 1;
        cursor_.v.addr &= 0x3FFF;
        return true;
    }

    return false;
}

void PPU::patterntable_img(Image<128, 128>& image, byte_t index, Palette const& palette) const
{
    for (int y = 0; y < 128; ++y)
    {
        for (int x = 0; x < 128; ++x)
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
            tile.ntbyte_ = load_(ntaddr | ntrow * 32 + ntcol);
            tile.atbyte_ = get_attribute_(ntaddr, ntrow / 2, ntcol / 2);
            tile.half_ = ppuctrl_.bg_pat_;

            address_t paladdr = bg_palette_addr[tile.atbyte_];
            Palette palette(
                g_palette[load_(0x3F00)],
                g_palette[load_(paladdr + 0)],
                g_palette[load_(paladdr + 1)],
                g_palette[load_(paladdr + 2)]);

            for (int trow = 0; trow < 8; ++trow)
            {
                for (int tcol = 0; tcol < 8; ++tcol)
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

void PPU::tick_()
{
    // skip one cycle on odd frame at scanline 261
    if (cycle_ < 340 && !(frame_ % 2 == 1 && scanline_ == 261 && cycle_ == 339))
    {
        ++cycle_;
    }
    else
    {
        cycle_ = 0;
        ++scanline_;

        if (scanline_ == 261)
        {
            scanline_ = -1;
            ++frame_;
        }
    }

    if (cycle_ == 1)
    {
        ppustatus_.sprite_0_hit_ = 0;
        ppustatus_.sprite_overflow_ = 0;

        // status checks
        if (scanline_ == -1)
        {
            ppustatus_.vblank_ = 0; // end of vblank
        }

        if (scanline_ == 241)
        {
            ppustatus_.vblank_ = 1; // start of vblank
            if (ppuctrl_.nmi_)
                bus_->cpu_.interrupt(true); // generate NMI
        }
    }

    if (scanline_ == -1 && cycle_ == 304)
    {
        if (ppumask_.render_bg_)
            cursor_.v.addr = (cursor_.v.addr & HORI_MASK) | (cursor_.t.addr & VERT_MASK);
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
                int trow = row % 8;
                int tcol = col % 8;

                auto lidx = (col % 16) / 8;
                auto& tile = bg_tiles_[lidx].read();

                bg_pixel = (tile.lpat_ >> (7 - tcol) & 1) + (((tile.hpat_ >> (7 - tcol)) & 1) << 1);
                bg_pal = tile.atbyte_;
            }

            if (ppumask_.render_fg_)
            {
                auto& sprites = secondary_oam_.read();

                for (int i = 0; i < sprites.count_; ++i)
                {
                    Sprite const& sprite = sprites.list_[i].sprite_;
                    if (sprite.x_ < col && (col - sprite.x_) <= 8) // TODO 8x16 sprites
                    {
                        byte_t pattern = sprite.tile_;
                        Tile tile = get_pattern_tile(pattern, ppuctrl_.fg_pat_);

                        fg_pal = sprite.att_ & 0x3;
                        byte_t flip = sprite.att_ >> 6;
                        has_sprite_0 = has_sprite_0 || sprites.list_[i].oam_idx_ == 0;

                        int ty = (row - sprite.y_ - 1);
                        int tx = (col - sprite.x_ - 1);

                        fg_priority = (sprite.att_ & 0x20) == 0;
                        fg_pixel = get_pixel(tile, tx, ty, flip);
                        if (fg_pixel != 0) break;
                    }
                }
            }

            bool render_fg = ppumask_.render_fg_ && fg_pixel != 0 && (bg_pixel == 0 || fg_priority);
            bool render_bg = ppumask_.render_bg_;
            bool sprite_0_eval = has_sprite_0 && ppumask_.render_fg_ && col < 255;

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

void PPU::bg_eval_()
{
    if (ppumask_.render_bg_)
    {
        auto& v = cursor_.v;

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
                    v.N ^= 0b10;
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

        if (cycle_ == 257)
        {
            // Copy Horizontal bits of t into v
            v.addr = (v.addr & VERT_MASK) | (cursor_.t.addr & HORI_MASK);
        }

        if (scanline_ >= -1 && scanline_ < 240)
        {
            if (cycle_ > 0 && cycle_ <= 256 || cycle_ > 320 && cycle_ <= 336)
            {
                uint16_t fetch = ((cycle_ - 1) % 16);
                auto lidx = fetch / 8;
                auto& tile = bg_tiles_[lidx].store();

                int op = fetch % 8;
                switch (op)
                {
                case 0: // NT byte
                {
                    address_t ntaddr = 0x2000 | (v.addr & 0x0FFF);
                    tile.ntbyte_ = load_(ntaddr);
                }
                break;

                case 2: // AT byte
                {
                    address_t ataddr = 0x23C0 | (v.addr & 0x0C00) | ((v.addr >> 4) & 0x38) | ((v.addr >> 2) & 0x07);
                    byte_t areashift = (((v.Y / 2) % 2) ? 4 : 0) + (((v.X / 2) % 2) ? 2 : 0);
                    tile.atbyte_ = (load_(ataddr) >> areashift) & 0x3;
                }
                break;

                case 4: // Pat (low + high)
                {
                    tile.half_ = ppuctrl_.bg_pat_;

                    address_t addr
                        = static_cast<address_t>(tile.half_ & 0x1) << 12
                        | static_cast<address_t>(tile.ntbyte_) << 4
                        | static_cast<address_t>(v.y & 0b111) << 0;

                    tile.lpat_ = load_(addr);
                    tile.hpat_ = load_(addr + 8);
                }
                break;

                case 7: // Horizontal increment + switch register latching
                {
                    if (v.X == 31)
                    {
                        v.X = 0;
                        v.N ^= 0b01;
                    }
                    else
                    {
                        v.X++;
                    }

                    auto& latch = bg_tiles_[lidx];
                    latch.flip();
                }
                break;
                }
            }
        }
    }
}

void PPU::fg_eval_()
{
    if (scanline_ >= -1 && scanline_ < 240)
    {
        if (cycle_ == 1)
        {
            secondary_oam_.flip();
            secondary_oam_.store().count_ = 0;
            secondary_oam_.store().list_.fill({0xFF, 0xFF, 0xFF, 0xFF});
        }

        if (cycle_ == 256)
        {
            for (int i = 0; i < 64; ++i)
            {
                Sprite& sprite = reinterpret_cast<Sprite*>(oam_.data())[i];
                int y = scanline_ + 1;

                if (sprite.y_ < y && (y - sprite.y_) <= 8)
                {
                    auto& sprites = secondary_oam_.store();
                    if (sprites.count_ < 8)
                    {
                        auto& entry = sprites.list_[sprites.count_];
                        entry.oam_idx_ = i;
                        entry.sprite_ = sprite;

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

byte_t PPU::load_(address_t addr) const
{
    byte_t value = 0;
    if (cart_->on_ppu_read(addr, value))
        return value;

    return memory_[mirror_addr_(addr)];
}

void PPU::store_(address_t addr, byte_t value)
{
    if (cart_->on_ppu_write(addr, value))
        return;
        
    memory_[mirror_addr_(addr)] = value;
}

address_t PPU::mirror_addr_(address_t addr) const
{
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
    case Mirroring::Vertical:
        if (addr >= 0x2800 && addr < 0x3000)
            addr -= 0x0800;
        break;

    case Mirroring::Horizontal:
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