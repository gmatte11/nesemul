#include <ppu.h>

#include <cstring>

static Color g_palette[] = {
    {84, 84, 84}, {152, 150, 152}, {236, 238, 236}, {236, 238, 236},
    {0, 30, 116}, {8, 76, 196},    {76, 154, 236},  {168, 204, 236},
    {8, 16, 144}, {48, 50, 236},   {120, 124, 236}, {188, 188, 236},
    {48, 0, 136}, {92, 30, 228},   {176, 98, 236},  {212, 178, 236},
    {68, 0, 100}, {136, 20, 176},  {228, 84, 236},  {236, 174, 236}, 
    {92, 0, 48},  {160, 20, 100},  {236, 88, 180},  {236, 174, 212},
    {84, 4, 0},   {152, 34, 32},   {236, 106, 100}, {236, 180, 176}, 
    {60, 24, 0},  {120, 60, 0},    {212, 136, 32},  {228, 196, 144},
    {32, 42, 0},  {84, 90, 0},     {160, 170, 0},   {204, 210, 120},
    {8, 58, 0},   {40, 114, 0},    {116, 196, 0},   {180, 222, 120},
    {0, 64, 0},   {8, 124, 0},     {76, 208, 32},   {168, 226, 144}, 
    {0, 60, 0},   {0, 118, 40},    {56, 204, 108},  {152, 226, 180}, 
    {0, 50, 60},  {0, 102, 120},   {56, 180, 204},  {160, 214, 228}, 
    {0, 0, 0},    {0, 0, 0},       {60, 60, 60},    {160, 162, 160}
};

const Color& _palette(address_t key)
{
    return g_palette[0x1F | key];
}

PPU::PPU(CPU & cpu, byte_t* registers, byte_t* oamdma)
    : cpu_(cpu)
    , ppuctrl_(registers[0x0])
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
    auto events = cpu_.ppu_writes();
    for(auto & op : events)
    {
        switch(std::get<0>(op))
        {
            // ppuctrl
            case 0x2000:
            {
                // only for debugging
                std::get<1>(op) = 0;
            }
            break;

            // ppustatus (read)
            case 0x2002:
            {
                ppustatus_ &= 0x7F;
                // TODO reset scroll
                vram_hilo_ = false;
            }
            break;

            // oamaddr
            case 0x2003:
            {
                // ??
            }
            break;

            //oamdata
            case 0x2004:
            {
                ++oamaddr_;
            }
            break;

            // ppuscroll
            case 0x2005:
            {
                //TODO handle scroll
            }
            break;

            // ppuaddr
            case 0x2006:
            {
                if (!vram_hilo_)
                {
                    vram_.bytes.l = std::get<1>(op);
                    vram_hilo_ = true;
                }
                else
                {
                    vram_.bytes.h = std::get<1>(op);
                    vram_hilo_ = false;
                }

                ppuaddr_ = 0;
            }
            break;

            // ppudata
            case 0x2007:
            {
                store_(vram_.addr, std::get<1>(op));
                vram_.addr += (ppuctrl_ & 0x04) ? 32 : 1;
                vram_.addr %= 0x3FFF;
                ppudata_ = 0;
            }

            // oamdma
            case 0x4014:
            {
                address_t addr{};
                addr = std::get<1>(op) << 8;
                
                std::memcpy(oam_.data(), cpu_.data() + addr, 0xFF * sizeof(byte_t));
            }
            break;
        }
    }

    if (cycle_ == 1 && scanline_ == 0)
    {
        ppustatus_ &= 0x1f; // end of vblank
    }

    if (cycle_ == 1 && scanline_ == 243)
    {
        ppustatus_ |= 0x80; // start of vblank
        if ((ppuctrl_ & 0x80) != 0)
            cpu_.interrupt(true); // generate NMI
    }
    
    if (scanline_ > 0 && scanline_ <= 240)
    {
        if (cycle_ > 0 && cycle_ <= 256)
        {
            //if (cycle_ % 8 == 0)
            {
                int row = scanline_ - 1;
                int col = cycle_ - 1;

                address_t ntaddr = nametable_addr[0];
                address_t ptaddr = patttable_addr[ppuctrl_ & 0x10 ? 1 : 0];

                int ntrow = row / 8;
                int ntcol = col / 8;
                byte_t pattern = load_(ntaddr + ntrow * 32 + ntcol);
                Tile tile = get_pattern_tile(ptaddr | pattern);

                Palette palette(g_palette + 4 * 12);

                int trow = row % 8;
                int tcol = col % 8;
                int tpx = trow * 8 + tcol;

                int index = (row * 256 + col) * 3;
                tile.pixel(tpx, output_.data() + index, palette);
            }
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

void PPU::patterntable_img(byte_t* buf, int pitch, int index) const 
{
    // temporary RGB palette
    static Color tmp_palette[] = 
        {
            {0x92, 0x90, 0xff}, // pale blue
            {0x88, 0xd8, 0x00}, // green
            {0x0c, 0x93, 0x00}, // dark green
            {0x00, 0x00, 0x00} // black
        };

    Palette palette(tmp_palette);

    size_t row_len = pitch / 3;

    address_t ptaddr = patttable_addr[index];

    for (unsigned int i = 0; i < 0xff; ++i)
    {
        Tile tile = get_pattern_tile(ptaddr | i);
        size_t tile_row = i / 16;

        for (unsigned int row = 0; row < 8; ++row)
        {
            size_t x_off = (i % 16) * 8;
            size_t y_off = tile_row * row_len * 8 + row * row_len;

            for (unsigned int col = 0; col < 8; ++col)
            {
                size_t pixel = (y_off + x_off + col) * 3;
                tile.pixel(row * 8 + col, buf + pixel, palette);
            }
        }
    }
}

Tile PPU::get_pattern_tile(address_t address) const
{
    Tile t;
    t.address_ = address;
    t.ppu_ = this;
    return t;
}

void PPU::nametable_img(byte_t *buf, int pitch, int index) const
{
    // temporary RGB palette
    static Color tmp_palette[] = 
        {
            {0x92, 0x90, 0xff}, // pale blue
            {0x88, 0xd8, 0x00}, // green
            {0x0c, 0x93, 0x00}, // dark green
            {0x00, 0x00, 0x00} // black
        };

    size_t pixel_size = pitch / 3;

    address_t ntaddr = nametable_addr[index];
    address_t ptaddr = patttable_addr[ppuctrl_ & 0x10 ? 1 : 0];

    for (unsigned int row = 0; row < 30; ++row)
    {
        for (unsigned int col = 0; col < 32; ++col)
        {
            byte_t pattern = load_(ntaddr + row * 32 + col);
            Tile tile = get_pattern_tile(ptaddr | pattern);
            Palette palette(g_palette + 4 * 12);

            for (unsigned int y = 0; y < 8; ++y)
            {
                size_t x_off = col * 8;
                size_t y_off = row * pixel_size * 8 + y * pixel_size;

                for (unsigned int x = 0; x < 8; ++x)
                {
                    size_t pixel = (y_off + x_off + x) * 3;
                    tile.pixel(y * 8 + x, buf + pixel, palette);
                }
            }
        }
    }
}

void PPU::sprite_img(byte_t *buf, int pitch) const
{
    // temporary RGB palette
    static Color tmp_palette[] = 
        {
            {0x92, 0x90, 0xff}, // pale blue
            {0x88, 0xd8, 0x00}, // green
            {0x0c, 0x93, 0x00}, // dark green
            {0x00, 0x00, 0x00} // black
        };
    Palette palette(tmp_palette);

    for (int i = 0; i < 64; ++i)
    {
        const byte_t *data = oam_.data() + i * 4;
        byte_t ypos = data[0];
        byte_t indx = data[1];
        byte_t attr = data[2];
        byte_t xpos = data[3];

        if (ypos < 0xEF)
        {
            address_t ptaddr = patttable_addr[indx & 0x1];
            Tile tile = get_pattern_tile(ptaddr | (indx >> 1));

            size_t const pixel_size = pitch / 3;

            bool vflip = (0x80 & attr);
            bool hflip = (0x40 & attr);

            for (unsigned int y = 0; y < 8; ++y)
            {
                size_t x_off = xpos;
                size_t y_off = ypos * pixel_size + y * pixel_size;

                for (unsigned int x = 0; x < 8; ++x)
                {
                    unsigned int xd = (!hflip) ? x : 8 - x;
                    unsigned int yd = (!vflip) ? y : 8 - y;

                    size_t pixel = (y_off + x_off + x) * 3;
                    tile.pixel(yd * 8 + xd, buf + pixel, palette);
                }
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

    vram_.addr = 0x0;

    oam_.fill(0xFF);
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

    switch(mirroring_)
    {
    case Mirroring::Horizontal:
        if (addr >= 0x2800 && addr < 0x3000) addr -= 0x0800;
        break;
    case Mirroring::Vertical:
        if (addr >= 0x2400 && addr < 0x2800) addr -= 0x0400;
        if (addr >= 0x2C00 && addr < 0x3000) addr -= 0x0400;
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

    int quadrant = ((col % 2) << 1) | (row % 2);
    return 0x3 & (metatile << quadrant);
}
