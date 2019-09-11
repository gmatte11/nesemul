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

const Color& _palette(address_t key)
{
    return g_palette[0x1F | key];
}

address_t nametable_addr[] = {0x2000, 0x2400, 0x2800, 0x2C00};
address_t patttable_addr[] = {0x0000, 0x1000};

void PPU::next()
{
    // status checks
    if (cycle_ == 1 && scanline_ == -1)
    {
        ppustatus_ &= 0x1f; // end of vblank
    }

    if (cycle_ == 1 && scanline_ == 241)
    {
        ppustatus_ |= 0x80; // start of vblank
        if ((ppuctrl_ & 0x80) != 0)
            bus_->cpu_.interrupt(true); // generate NMI
    }

    // rendering    
    if (scanline_ >= 0 && scanline_ < 240)
    {
        if (cycle_ > 0 && cycle_ <= 256)
        {
            {
                int row = scanline_;
                int col = cycle_ - 1;

                int trow = row % 8;
                int tcol = col % 8;

                auto lidx = (col % 16) / 8;
                //if (tcol == 0)
                //bg_tiles_[lidx].flip();

                auto& tile = bg_tiles_[lidx].read();

                static address_t bg_palette_addr[] = {0x3F01, 0x3F05, 0x3F09, 0x3F0D};
                address_t paladdr = bg_palette_addr[tile.atbyte_];
                Palette palette(
                    g_palette[load_(0x3F00)], 
                    g_palette[load_(paladdr + 0)],
                    g_palette[load_(paladdr + 1)], 
                    g_palette[load_(paladdr + 2)]);

                byte_t pixel = tile.pixel(tcol, trow);
                output_.set(col, row, palette.get(pixel));
            }

            {
                auto& sprites = secondary_oam_.read();

                for (int i = 0; i < sprites.count_; ++i)
                {
                    int row = scanline_;
                    int col = cycle_ - 1;

                    Sprite const& sprite = sprites.list_[i];
                    if (sprite.x_ < col && (col - sprite.x_) < 8) // TODO 8x16 sprites
                    {
                        byte_t pattern = sprite.tile_;
                        Tile tile = get_pattern_tile(pattern, ppuctrl_ & 0x08 ? 1 : 0);
                        
                        byte_t pal = sprite.att_ & 0x3;
                        byte_t flip = sprite.att_ >> 5;

                        static address_t sprite_palette_addr[] = {0x3F11, 0x3F15, 0x3F19, 0x3F1D};
                        address_t paladdr = sprite_palette_addr[pal];
                        Palette palette(
                            {0xFF, 0xFF, 0xFF, 0x00},
                            g_palette[load_(paladdr + 0)],
                            g_palette[load_(paladdr + 1)],
                            g_palette[load_(paladdr + 2)]);

                        int ty = (row - sprite.y_ - 2);
                        int tx = (col - sprite.x_ - 1);
                        byte_t pixel = tile.pixel(tx, ty, flip);
                        if (pixel != 0 && (sprite.att_ & 0x20) == 0)
                            output_.set(col, row, palette.get(pixel));
                    }
                }
            }
        }
    }

    // BG latching
    if (scanline_ >= -1 && scanline_ < 240)
    {
        if (cycle_ > 0 && cycle_ <= 256 || cycle_ > 320 && cycle_ <= 336)
        {
            int ntrow = (cycle_ < 321) ? (scanline_) / 8 : (scanline_ + 1) / 8;
            int ntcol = (cycle_ < 321) ? ((cycle_ - 1) / 8) + 2 : (cycle_ - 321) / 8;

            uint16_t fetch = ((cycle_ - 1) % 16);
            auto lidx = fetch / 8;
            auto& tile = bg_tiles_[lidx].store();
            tile.ppu_ = this;

            int op = fetch % 8;
            switch (op)
            {
            case 0: // NT byte
            {
                address_t ntaddr = nametable_addr[0]; // TODO: SCROLLING
                tile.ntbyte_ = load_(ntaddr | ntrow * 32 + ntcol);
            }
            break;

            case 2: // AT byte
            {
                address_t ntaddr = nametable_addr[0]; // TODO: SCROLLING
                tile.atbyte_ = get_attribute_(ntaddr, ntrow / 2, ntcol / 2);
            }
            break;

            case 4: // Pat (low)
            {
                tile.half_ = ppuctrl_ & 0x10 ? 1 : 0;
            }
            break;

            case 7: // H/V inc
            {
                auto& latch = bg_tiles_[lidx];
                latch.flip();
            }
            break;
            }
        }
            }

            // sprite eval
    if (scanline_ >= 0 && scanline_ < 240)
    {
            if (cycle_ == 1)
            {
                secondary_oam_.flip();
                secondary_oam_.store().count_ = 0;
                secondary_oam_.store().list_.fill({0xFF, 0xFF, 0xFF, 0xFF});
            }

            if (cycle_ > 64 && cycle_ <= 256)
            {
                if (cycle_ % 2 == 1)
                {
                    auto& sprites = secondary_oam_.store();
                    int n = (cycle_ - 65) / 2;
                    Sprite* sprite = reinterpret_cast<Sprite*>(oam_.data() + 4 * n);

                    if (sprite->y_ < scanline_ && (scanline_ - sprite->y_) < 8)
                    {
                        if (sprites.count_ < 8)
                        {
                            sprites.list_[sprites.count_] = *sprite;
                            ++sprites.count_;
                        }
                        else
                        {
                            // TODO Sprite overflow bug
                            ppustatus_ |= 0x20;
                        }
                    }
                }
            }
    }

    // skip one cycle on odd frame at scanline 261
    if (cycle_ < 340 && !(frame_ % 2 == 1 && scanline_ == 261 && cycle_ == 339))
    {
        ++cycle_;
    }
    else
    {
        cycle_ = 0;
        ++scanline_;

        if (scanline_ == 262)
        {
            scanline_ = 0;
            ++frame_;
        }
    }
}

void PPU::reset()
{
    scanline_ = -1;
    cycle_ = 0;
    frame_ = 0;

    ppuctrl_ = 0x0;
    ppumask_ = 0x0;
    ppuscroll_ = 0x0;
    ppudata_ = 0x0;

    vram_.addr = 0x0;

    oam_.fill(0xFF);
}

bool PPU::on_write(address_t addr, byte_t value)
{
    switch (addr)
    {
        case 0x2000: 
            ppuctrl_ = value;
            return true;

        case 0x2001:
            ppumask_ = value;
            return true;

            // oamaddr
        case 0x2003:
            // TODO
            return true;

            //oamdata
        case 0x2004:
            // TODO
            ++oamaddr_;
            return true;

            // ppuscroll
        case 0x2005:
            //TODO handle scroll
            return true;

            // ppuaddr
        case 0x2006:
            {
                if (!vram_hilo_)
                {
                    vram_.bytes.l = value;
                    vram_hilo_ = true;
                }
                else
                {
                    vram_.bytes.h = value;
                    vram_hilo_ = false;
                }

                ppuaddr_ = 0;
            }
            return true;

            // ppudata
        case 0x2007:
            {
                store_(vram_.addr, value);
                vram_.addr += (ppuctrl_ & 0x04) ? 32 : 1;
                vram_.addr %= 0x3FFF;
                ppudata_ = 0;
            }
            return true;

            // oamdma
        case 0x4014:
            {
                address_t addr = value << 8;
                bus_->ram_.memcpy(oam_.data(), addr, 0xFF * sizeof(byte_t));
            }
            return true;
    }

    return false;
}

bool PPU::on_read(address_t addr, byte_t& value)
{
    switch (addr)
    {
        case 0x2002:
            value = ppustatus_;
            ppustatus_ &= 0x7F;
            // TODO reset scroll
            vram_hilo_ = false;
            return true;

        case 0x2004:
            value = oamdata_;
            return true;

        case 0x2007:
            value = ppudata_;
            vram_.addr += (ppuctrl_ & 0x04) ? 32 : 1;
            vram_.addr %= 0x3FFF;
            ppudata_ = 0;
            return true;
    }

    return false;
}

void PPU::patterntable_img(Image<128, 128>& image, byte_t index) const 
{
    // temporary RGB palette
    static Color tmp_palette[] = {
            {0x92, 0x90, 0xff}, // pale blue
            {0x88, 0xd8, 0x00}, // green
            {0x0c, 0x93, 0x00}, // dark green
            {0x00, 0x00, 0x00} // black
        };

    Palette palette(tmp_palette);

    for (int y = 0; y < 128; ++y)
    {
        for (int x = 0; x < 128; ++x)
        {
            byte_t tx = x / 8;
            byte_t ty = y / 8;
            byte_t ntbyte = (ty << 4 | tx);

            Tile tile = get_pattern_tile(ntbyte, index);
            byte_t pixel = tile.pixel(x % 8, y % 8);
            image.set(x, y, palette.get(pixel));
            }
        }
    }

Tile PPU::get_pattern_tile(byte_t ntbyte, byte_t half) const
{
    Tile t;
    t.ntbyte_ = ntbyte;
    t.half_ = half & 0x1;
    t.ppu_ = this;
    return t;
}

byte_t PPU::load_(address_t addr) const
{
    return memory_[mirror_addr_(addr)];
}

void PPU::store_(address_t addr, byte_t value)
{
    memory_[mirror_addr_(addr)] = value;
}

address_t PPU::mirror_addr_(address_t addr) const
{
    // $3F20-$3FFF mirrors $3F00-$3F1F
    //TODO if (addr >= 0x3F20 && addr < 0x4000) 

    // $3000-$3EFF mirrors $2000-$2EFF;
    if (addr >= 0x3000 && addr < 0x3F00) addr -= 0x1000;

    switch (mirroring_)
    {
    case Mirroring::Horizontal:
        if (addr >= 0x2800 && addr < 0x3000) addr -= 0x0800;
        break;
    case Mirroring::Vertical:
        if (addr >= 0x2400 && addr < 0x2800) addr -= 0x0400;
        if (addr >= 0x2C00 && addr < 0x3000) addr -= 0x0400;
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

byte_t Tile::pixel(uint8_t x, uint8_t y, byte_t flip /*= 0*/) const
{
    if (0b10 & flip) x = 7 - x; //vert flip
    if (0b01 & flip) y = 7 - y; //hori flip

    address_t addr =
        static_cast<address_t>(half_ & 0x1) << 12 |
        static_cast<address_t>(ntbyte_) << 4 |
        static_cast<address_t>(y & 0b111) << 0;

    byte_t lpat = ppu_->load(addr);
    byte_t hpat = ppu_->load(addr + 8);

    byte_t mask = 1 << (7 - x);
    byte_t pixel = 
        (((hpat & mask) != 0) ? 0b10 : 0) |
        (((lpat & mask) != 0) ? 0b01 : 0);

    return pixel;
}