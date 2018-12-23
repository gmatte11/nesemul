#ifndef __SDL_RENDERER_H__
#define __SDL_RENDERER_H__
#include <SDL2/SDL.h>

#include <image.h>
#include <ppu.h>
#include <vector>
#include <functional>

class Viewport
{
public:
    Viewport(SDL_Window* window);
    ~Viewport();

    void init_texture(int width, int height);
    SDL_Texture* get_texture() const { return tex_; }

    void flip();

private:
    SDL_Texture* tex_;
    SDL_Renderer* renderer_;
    int w_;
    int h_;
};

class SDLRenderer
{
public:
    SDLRenderer();
    ~SDLRenderer();

    bool update(const PPU& ppu);
    bool timeout();

private:
    void draw(const PPU& ppu);

    Viewport* init_window(int width, int height);

    std::vector<std::pair<SDL_Window*, Viewport*>> viewports_;
    std::vector<std::pair<std::function<void(SDL_Texture*, const PPU &)>, SDL_Texture*>> resources_;
    Uint32 lastUpdate_;
};

#endif // __SDL_RENDERER_H__
