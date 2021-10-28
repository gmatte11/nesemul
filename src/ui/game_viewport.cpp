#include "game_viewport.h"

#include "ppu.h"

GameViewport::GameViewport()
{
    texture_.create(256, 240);
    setTexture(&texture_, true);
    setSize({256.f, 240.f});
}

void GameViewport::update(PPU const& ppu)
{
    texture_.update(ppu.output().data());
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