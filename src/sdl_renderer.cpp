#include <sdl_renderer.h>

#include <cstring>

void draw_naive(SDL_Texture* tex, const PPU & ppu)
{
    void* pixels;
    int pitch = 0;

    SDL_LockTexture(tex, nullptr, &pixels, &pitch);
    ppu.nametable_img((byte_t*)pixels, pitch, 0);
    SDL_UnlockTexture(tex);
}

void draw_nam(SDL_Texture* tex, const PPU & ppu)
{
    void* pixels;
    int pitch = 0;

    SDL_Rect rects[] = {
        {0  , 0  , 256, 240},
        {256, 0  , 256, 240},
        {0  , 240, 256, 240},
        {256, 240, 256, 240}
    };

    for (int i = 0; i < 4; ++i)
    {
        SDL_LockTexture(tex, &rects[i], &pixels, &pitch);
        ppu.nametable_img((byte_t*)pixels, pitch, i);
        SDL_UnlockTexture(tex);
    }
}

void draw_pat(SDL_Texture* tex, const PPU & ppu)
{
    void* pixels;
    int pitch = 0;

    // PAT 0000
    SDL_Rect rect = {0, 0, 128, 128};
    SDL_LockTexture(tex, &rect, &pixels, &pitch);
    ppu.patterntable_img((byte_t*)pixels, pitch, 0);
    SDL_UnlockTexture(tex);

    // PAT 1000
    rect.y += 128 + 10;
    SDL_LockTexture(tex, &rect, &pixels, &pitch);
    ppu.patterntable_img((byte_t*)pixels, pitch, 1);
    SDL_UnlockTexture(tex);
}

void draw_oam(SDL_Texture* tex, const PPU & ppu)
{   
    void* pixels;
    int pitch = 0;

    SDL_LockTexture(tex, nullptr, &pixels, &pitch);
    ppu.sprite_img((byte_t*)pixels, pitch);
    SDL_UnlockTexture(tex);
}

Viewport::Viewport(SDL_Window* window)
{
    if ((renderer_ = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == nullptr)
        throw std::runtime_error("Can't initialize SDL renderer.");
}

Viewport::~Viewport()
{
    if (tex_ != nullptr)
    {
        SDL_DestroyTexture(tex_);
    }

    SDL_DestroyRenderer(renderer_);
}

void Viewport::init_texture(int width, int height)
{
    if (tex_ != nullptr)
    {
        SDL_DestroyTexture(tex_);
    }

    tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void Viewport::flip()
{
    SDL_SetRenderDrawColor(renderer_, 0x0, 0x0, 0x0, 0xFF);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, tex_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

SDLRenderer::SDLRenderer()
    : lastUpdate_(0ul)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Can't initialize SDL.");

    Viewport *v;

    // Main game
    v = init_window("NESEMUL", 256*2, 240*2);
    v->init_texture(256, 240);
    resources_.emplace_back(std::make_pair(draw_naive, v->get_texture()));

    // Name table (debug)
    v = init_window("Nametables", 512, 480);
    v->init_texture(512, 480);
    resources_.emplace_back(std::make_pair(draw_nam, v->get_texture()));

    // Pattern table (debug)
    v = init_window("Pattern tables", 128*2, (128*2+10)*2);
    v->init_texture(128, 128*2+10);
    resources_.emplace_back(std::make_pair(draw_pat, v->get_texture()));
}

SDLRenderer::~SDLRenderer()
{
    for(auto & v : viewports_)
    {
        delete v.second;
        SDL_DestroyWindow(v.first);
    }
    SDL_Quit();
}

bool SDLRenderer::update(const PPU& ppu)
{
    static SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            return false;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                return false;
        }
    }

    draw(ppu);

    lastUpdate_ = SDL_GetTicks();
    return true;
}

void SDLRenderer::draw(const PPU& ppu)
{
    for (auto & r : resources_)
    {
        r.first(r.second, ppu);
    }

    for(auto & v : viewports_)
    {
        v.second->flip();
    }
}

Viewport* SDLRenderer::init_window(const char* name, int width, int height)
{
    SDL_Window* window;
    if ((window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width , height, SDL_WINDOW_RESIZABLE)) == nullptr)
        throw std::runtime_error("Can't initialize SDL window.");

    return viewports_.emplace_back(std::make_pair(window, new Viewport{window})).second;
}

bool SDLRenderer::timeout()
{
    return (SDL_GetTicks() - lastUpdate_ >= 1000 / 30);
}
