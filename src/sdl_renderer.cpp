#include <sdl_renderer.h>

#include <cstring>

SDLRenderer::SDLRenderer(int width, int height)
    : lastUpdate_(0ul)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Can't initialize SDL.");

    if ((window_ = SDL_CreateWindow("NESEMUL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * 3, height * 3, SDL_WINDOW_MINIMIZED)) == nullptr)
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
    return true;
}

void SDLRenderer::draw(const PPU& ppu)
{
    static void* pixels;
    static int pitch = 0;

    // NAM (todo)
    SDL_Rect rect{0, 0, 256, 256};
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    for (int i = 0; i < pitch / 3 * rect.h; ++i)
    {
        std::memcpy(static_cast<byte_t *>(pixels) + (i * 3), std::array<byte_t, 3>{0x92, 0x90, 0xff}.data(), 3);
    }
    SDL_UnlockTexture(display_);

    // PAT 0000
    rect = {32 * 8 + 10, 0, 128, 128};
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    ppu.pattern_table((byte_t*)pixels, pitch, 0);
    SDL_UnlockTexture(display_);

    // PAT 1000
    rect.y += 128 + 10;
    SDL_LockTexture(display_, &rect, &pixels, &pitch);
    ppu.pattern_table((byte_t*)pixels, pitch, 1);
    SDL_UnlockTexture(display_);

    SDL_RenderCopy(renderer_, display_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

bool SDLRenderer::timeout()
{
    return (SDL_GetTicks() - lastUpdate_ >= 1000 / 30);
}
