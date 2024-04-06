#include "ppu_utils.h"

#include "emulator.h"
#include "bus.h"

#include <SFML/Graphics.hpp>

Palette ppu_read_palette(const BUS& bus, int idx)
{
    NES_ASSERT(idx >= 0 && idx < 8);
    if (idx < 4)
    {
        static constexpr address_t palette_bg[] = {0x3F01, 0x3F05, 0x3F09, 0x3F0D};
        address_t paladdr = palette_bg[idx];
        return Palette(
            g_palette[bus.read_ppu(0x3F00)],
            g_palette[bus.read_ppu(paladdr + 0)],
            g_palette[bus.read_ppu(paladdr + 1)],
            g_palette[bus.read_ppu(paladdr + 2)]
        );
    }
    else
    {
        static constexpr address_t palette_fg[] = {0x3F11, 0x3F15, 0x3F19, 0x3F1D};
        address_t paladdr = palette_fg[idx - 4];
        return Palette(
            {0xFF, 0xFF, 0xFF, 0x00},
            g_palette[bus.read_ppu(paladdr + 0)],
            g_palette[bus.read_ppu(paladdr + 1)],
            g_palette[bus.read_ppu(paladdr + 2)]
        );
    }
}

// extract a single pixel value [0-3] from tile data
byte_t ppu_tile_pixel(const BUS& bus, Tile tile, int x, int y, byte_t flip_mask = 0)
{
    if (0b01 & flip_mask) x = 7 - x; //hori flip
    if (0b10 & flip_mask) y = 7 - y; //vert flip

    address_t addr = 
        static_cast<address_t>(tile.half_ & 0x1) << 12 |
        static_cast<address_t>(tile.ntbyte_) << 4 |
        static_cast<address_t>(y & 0b111) << 0;

    byte_t lpat = bus.read_ppu(addr);
    byte_t hpat = bus.read_ppu(addr + 8);

    byte_t mask = 1 << (7 - x);
    byte_t pixel = 
        (((hpat & mask) != 0) ? 0b10 : 0) | 
        (((lpat & mask) != 0) ? 0b01 : 0);

    return pixel;
}

void ui::ppu_patterntable_texture(sf::Texture& tex, std::span<const byte_t> chr_buf, const Palette& palette)
{
    NES_ASSERT((tex.getSize() == sf::Vector2u{128u, 128u}));

    for (int t = 0; t < 256; ++t)
    {
        TileImage tile;

        for (int row = 0; row < 8; ++row)
        {
            const int addr = ((0xFF & t) << 4) + (row & 0b111);
            const byte_t plane1 = chr_buf[addr];
            const byte_t plane2 = chr_buf[addr + 8];

            for (int col = 0; col < 8; ++col)
            {
                const byte_t pixel_mask = 1 << (7 - col);

                int color_idx = ((plane1 & pixel_mask) ? 0b10 : 0) 
                                | ((plane2 & pixel_mask) ? 0b01 : 0);

                tile.set(col, row, palette.get(color_idx));
            }
        }

        const int tx = t % 16;
        const int ty = t / 16;
        tex.update(tile.data(), 8, 8, tx * 8, ty * 8);
    }
}

byte_t ppu_read_tile_attributes(const BUS& bus, address_t ntaddr, int row, int col)
{
    // The attribute table is located at the end of the nametable (offset $03C0)
    address_t ataddr = ntaddr + 0x03C0;

    // Offset of the requested byte
    ataddr += static_cast<address_t>((row / 2) * 0x8 + (col / 2));

    byte_t metatile = bus.read_ppu(ataddr);

    int quadrant = 
        ((row % 2 == 1) ? 0b10 : 0) | 
        ((col % 2 == 1) ? 0b01 : 0);
        
    return 0b11 & (metatile >> (quadrant * 2));
}

void ppu_fill_nametable_tile(NAMImage& image, const BUS& bus, Tile tile, int ntcol, int ntrow, const Palette& palette)
{
    for (uint8_t row = 0; row < 8; ++row)
    {
        for (uint8_t col = 0; col < 8; ++col)
        {
            int x = (ntcol * 8) + col;
            int y = (ntrow * 8) + row;

            byte_t pixel = ppu_tile_pixel(bus, tile, col, row);
            image.set(x, y, palette.get(pixel));
        }
    }
}

void ppu_fill_nametable_image(NAMImage& image, address_t ntaddr)
{
    static constexpr address_t bg_palette_addr[] = {0x3F01, 0x3F05, 0x3F09, 0x3F0D};

    Emulator& emulator = *Emulator::instance();
    const BUS& bus = *emulator.get_bus();
    const PPU& ppu = *emulator.get_ppu();

    Tile tile;

    for (int ntrow = 0; ntrow < (240 / 8); ++ntrow)
    {
        for (int ntcol = 0; ntcol < (256 / 8); ++ntcol)
        {
            tile.ntbyte_ = bus.read_ppu(ntaddr | static_cast<address_t>(ntrow * 32 + ntcol));
            tile.atbyte_ = ppu_read_tile_attributes(bus, ntaddr, ntrow / 2, ntcol / 2);
            tile.half_ = ppu.get_background_half();

            Palette palette = ppu_read_palette(bus, tile.atbyte_);

            ppu_fill_nametable_tile(image, bus, tile, ntcol, ntrow, palette);
        }
    }
}

void ui::ppu_nametable_texture(sf::Texture &tex)
{
    static constexpr address_t nametable_addr[] = {0x2000, 0x2400, 0x2800, 0x2C00};
    NES_ASSERT(tex.getSize().x >= NAMImage::Width * 2 && tex.getSize().y >= NAMImage::Height * 2);

    NAMImage image;

    for (int i = 0; i < 4; ++i)
    {
        const address_t ntaddr = nametable_addr[i];
        ppu_fill_nametable_image(image, ntaddr);
    
        const int x = (i % 2) * NAMImage::Width;
        const int y = (i / 2) * NAMImage::Height;

        tex.update(image.data(), NAMImage::Width, NAMImage::Height, x, y);
    }
}

OAMSprite ppu_read_sprite_image(SpriteImage& image, const BUS& bus, int oam_idx)
{
    const PPU& ppu = bus.ppu_;

    OAMSprite sprite = reinterpret_cast<const OAMSprite*>(ppu.oam_data())[oam_idx];

    Tile tile;
    tile.ntbyte_ = sprite.tile_;
    tile.half_ = bus.ppu_.get_foreground_half() != 0;

    Palette palette = ppu_read_palette(bus, static_cast<int>(sprite.att_ & 0x3) + 4);

    for (uint8_t y = 0; y < 8; ++y)
    {
        for (uint8_t x = 0; x < 8; ++x)
        {
            byte_t pixel = ppu_tile_pixel(bus, tile, x, y, sprite.att_ >> 6);
            image.set(x, y, palette.get(pixel));
        }
    }

    return sprite;
}

void ui::ppu_oam_texture(sf::Texture& tex, std::span<OAMSprite> sprites)
{
    Emulator &emulator = *Emulator::instance();
    BUS &bus = *emulator.get_bus();

    SpriteImage image;

    for (int i = 0; i < 64; ++i)
    {
        sprites[i] = ppu_read_sprite_image(image, bus, i);

        const int x = (i % 8) * 8;
        const int y = (i / 8) * 8;

        tex.update(image.data(), 8, 8, x, y);
    }
}