#include "game_viewport.h"

#include "emulator.h"
#include "ppu.h"

constexpr int line_buf_size = 256 * 4;

consteval auto red_line()
{
    sf::Uint8 color[4] { 255, 0, 0, 255 };
    std::array<sf::Uint8, line_buf_size> arr{};

    for (int i = 0; i < arr.size(); ++i)
        arr[i] = color[i % 4];

    return arr;
}

GameViewport::GameViewport()
{
    texture_.create(256, 240);
    setTexture(&texture_, true);
    setSize({256.f, 240.f});
}

void GameViewport::update(PPU const& ppu)
{
    texture_.update(ppu.output().data());

    Emulator* emulator = Emulator::instance();
    if (emulator->is_debugging())
    {
        auto scanline = ppu.get_state().scanline_;
        if (scanline >= 0 && scanline < 240)
        {
            static constexpr auto line = red_line();
            texture_.update(line.data(), 256, 1, 0, scanline);
        }
    }
}

NametableViewport::NametableViewport()
{
    texture_.create(512, 480);
    setTexture(&texture_, true);
    setSize({512.f, 480.f});
}

void NametableViewport::update(PPU const& ppu)
{
    Image<256, 240> image;
    for (byte_t i = 0; i < 4; ++i)
    {
        ppu.nametable_img(image, i);

        int x = (i % 2 == 0) ? 0 : 256;
        int y = (i / 2 == 0) ? 0 : 240;
        texture_.update(image.data(), 256, 240, x, y);
    }
}