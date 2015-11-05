#include <ppu.h>

#include <cstring>

struct Color
{
    byte_t r, g, b;
};

static std::array<Color, 0x40> gPalette /*= {
    {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0}, {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0},
    {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0}, {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40}, {0, 102, 120}, {0, 0, 0},
    {236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32}, {160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60},
    {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}}*/;

const Color& _palette(address_t key)
{
    return gPalette[0x1F | key];
}

PPU::PPU(byte_t* registers, byte_t* oamdma)
    : ppuctrl_(registers[0x0])
    , ppumask_(registers[0x1])
    , ppustatus_(registers[0x2])
    , oamaddr_(registers[0x3])
    , oamdata_(registers[0x4])
    , ppuscroll_(registers[0x5])
    , ppuaddr_(registers[0x6])
    , ppudata_(registers[0x7])
    , oamdma_(*oamdma)
{
}

address_t nametable_addr[] = {0x2000, 0x2400, 0x2800, 0x2C00};
address_t patttable_addr[] = {0x0000, 0x1000};

void PPU::next()
{
    if (cycle_ % 89342 == 0 && (0x80 | ppuctrl_) != 0)
        ; // generate NMI

    address_t ntaddr = nametable_addr[0x03 & ppuctrl_];
    address_t ataddr = ntaddr + 0x03C0;
    address_t ptaddr = patttable_addr[0x10 | ppuctrl_];
    byte_t attr;
    byte_t name;
    byte_t lpat;
    byte_t hpat;

    /*for (int row = 0; row <= 240; ++row)
    {
        for (int col = 0; col < 256; ++col)
        {
            int pix = row * 256 + col;
            if (col % 8 == 0)
            {
                byte_t index = col \ 8;
                name = memory_[ntaddr + index];
                attr = memory_[ataddr + index];
                lpat = memory_[ptaddr + index];
                hpat = memory_[ptaddr + index + 8];
            }

            address_t pal_addr = 0;
            std::memset(output_.data() + pix * 3, &_palette(pal_addr), sizeof(Color));
        }
    }*/

    static union
    {
        address_t addr;
        struct
        {
            byte_t h;
            byte_t l;
        } bytes;
    } vram{};

    static bool addr_set = false;

    //for (int c = 0; c < 3; ++c)
    {
        ppustatus_ = 0xA0;

        if (ppuaddr_ != 0)
        {
            if (!addr_set)
            {
                vram.bytes.h = ppuaddr_;
                addr_set = true;
            }
            else
            {
                vram.bytes.l = ppuaddr_;
                addr_set = false;
            }

            ppuaddr_ = 0;
        }

        if (ppudata_ != 0)
        {
            memory_[vram.addr] = ppudata_;
            vram.addr += (ppuctrl_ & 0x4) ? 32 : 1;
            vram.addr %= 0x3fff;
            ppudata_ = 0;
        }

        /*if ((scanline_ >= 0 && scanline_ < 240) || scanline_ == 261)
        {
            if (cycle_ > 0 && cycle_ <= 256)
                ; // Data fetch
            // on cycle 0 and every 8 cycles (9, 17, 25, ...) the shift registers are refreshed
            // each 2 cycles (1 per bit) pixel data is read from shift registers and commited into tile.

            if (cycle_ > 256 && cycle_ <= 320)
                ; // Next scanline data fetch

            if (cycle_ > 320 && cycle_ <= 336)
                ; // Next scanline first two tiles are put in shift registers

            if (cycle_ > 336 && cycle_ <= 340)
                ; // two bytes fetch
        }
        else if (scanline_ == 241)
        {
            if (cycle_ == 1)
                ; // set VBlank flag
        }*/

        if (cycle_ < 340)
            ++cycle_;
        else
        {
            cycle_ = 0;
            ++scanline_;

            if (scanline_ == 262)
                scanline_ = 0;
        }
    }
}

void PPU::pattern_table(byte_t* buf, int pitch, int index) const
{
    size_t row_len = pitch / 3;

    address_t ptaddr = patttable_addr[index];
    
    byte_t lpat;
    byte_t hpat;

    for (unsigned int i = 0; i < 0xff; ++i)
    {
        size_t tile = i;
        size_t tile_row = tile / 16;

        for (unsigned int row = 0; row < 8; ++row)
        {
            address_t laddr = ptaddr + (i << 4) + row;
            address_t haddr = laddr + 8;
            lpat = memory_[laddr];
            hpat = memory_[haddr];

            size_t x_off = (tile % 16) * 8;
            size_t y_off = tile_row * row_len * 8 + row * row_len;

            for (unsigned int col = 0; col < 8; ++col)
            {
                size_t pixel = (y_off + x_off + col) * 3;
                byte_t val = (0x1 & (lpat >> (7 - col))) | ((0x1 & (hpat >> (7 - col))) << 1);

                // temporary RGB pallet
                static std::array<byte_t, 12> tmp_pallet = 
                    {
                        0x92, 0x90, 0xff, // pale blue
                        0x88, 0xd8, 0x00, // green
                        0x0c, 0x93, 0x00, // dark green
                        0x00, 0x00, 0x00 // black
                    };

                std::memcpy(buf + pixel, tmp_pallet.data() + (val * 3), 3);
            }
        }
    }
}

void PPU::reset()
{
    scanline_ = 261;
    cycle_ = 0;

    ppuctrl_ = 0x0;
    ppumask_ = 0x0;
    ppuscroll_ = 0x0;
    ppudata_ = 0x0;
}
