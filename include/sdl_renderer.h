#ifndef __SDL_RENDERER_H__
#define __SDL_RENDERER_H__
#include <SDL2/SDL.h>

#include <image.h>
#include <ppu.h>

class SDLRenderer
{
public:
    SDLRenderer(int width, int height);
    ~SDLRenderer();

    bool update();
    void draw(const PPU& ppu);
    bool timeout();

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* display_;
    Uint32 lastUpdate_;
};

#endif // __SDL_RENDERER_H__
