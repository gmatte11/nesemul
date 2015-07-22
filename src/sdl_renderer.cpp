#include <sdl_renderer.h>

#include <cstring>

SDLRenderer::SDLRenderer(int width, int height)
    : lastUpdate_(0ul)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Can't initialize SDL.");

    if ((window_ = SDL_CreateWindow("NESEMUL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0)) == nullptr)
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

void SDLRenderer::draw(const Image& img)
{
    static void* pixels;
    static int pitch;
    SDL_LockTexture(display_, NULL, &pixels, &pitch);
    std::memcpy(pixels, img.data(), img.size());
    SDL_UnlockTexture(display_);
    SDL_RenderCopy(renderer_, display_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

bool SDLRenderer::timeout()
{
    return (SDL_GetTicks() - lastUpdate_ >= 1000 / 30);
}
