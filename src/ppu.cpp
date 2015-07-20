#include <ppu.h>

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

void PPU::next()
{
    for (int c = 0; c < 3; ++c)
    {
        if ((scanline_ >= 0 && scanline_ < 240) || scanline_ == 261)
        {
            if (cycle_ > 0 && cycle_ <= 256)
                ; // Data fetch

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
        }

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

void PPU::reset()
{
    scanline_ = 261;
    cycle_ = 0;

    ppuctrl_ = 0x0;
    ppumask_ = 0x0;
    ppuscroll_ = 0x0;
    ppudata_ = 0x0;
}
