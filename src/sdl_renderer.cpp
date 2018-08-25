#include <sdl_renderer.h>

#include <cstring>

SDLRenderer::SDLRenderer(int width, int height)
    : lastUpdate_(0ul)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Can't initialize SDL.");

    if ((window_ = SDL_CreateWindow("NESEMUL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * 3, height * 3, 0)) == nullptr)
        throw std::runtime_error("Can't initialize SDL window.");

    if ((renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED)) == nullptr)
        throw std::runtime_error("Can't initialize SDL renderer.");

    display_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
}

SDLRenderer::~SDLRenderer()
{
    SDL_DestroyTexture(display_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool SDLRenderer::update()
{
    static SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            return false;
        }
    }

    lastUpdate_ = SDL_GetTicks();
    SDL_Delay((1000 / CLOCKS_PER_SEC) / 30);
    return true;
}

void SDLRenderer::draw(const PPU& ppu)
{
    static void* pixels;
    static int pitch = 0;

    // NAM
    SDL_Rect rect{0, 0, 256, 256};
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    ppu.nametable_img((byte_t*)pixels, pitch, 0);
    SDL_UnlockTexture(display_);

    // OAM
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    ppu.sprite_img((byte_t*)pixels, pitch);
    SDL_UnlockTexture(display_);

    // PAT 0000
    rect = {32 * 8 + 10, 0, 128, 128};
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    ppu.patterntable_img((byte_t*)pixels, pitch, 0);
    SDL_UnlockTexture(display_);

    // PAT 1000
    rect.y += 128 + 10;
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    ppu.patterntable_img((byte_t*)pixels, pitch, 1);
    SDL_UnlockTexture(display_);

    SDL_SetRenderDrawColor(renderer_, 0x0, 0x0, 0x0, 0xFF);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, display_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

bool SDLRenderer::timeout()
{
    return (SDL_GetTicks() - lastUpdate_ >= 1000 / 30);
}
