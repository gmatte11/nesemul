#pragma once

#include "types.h"
#include <SFML/Graphics.hpp>

namespace sf
{
    class RenderWindow;
}

class BUS;
class CPU;
class PPU;
class Emulator;

class SFMLRenderer
{
public:
    SFMLRenderer(Emulator* emulator);
    ~SFMLRenderer();
    
    bool update();
    bool timeout();

private:
    void draw();
    void draw_game(PPU const& ppu);
    void draw_pat(PPU const& ppu);
    void draw_pal(PPU const& ppu);
    void draw_oam(PPU const& ppu);
    void draw_asm(CPU const& cpu);
    void draw_nam(PPU const& ppu, sf::Texture* tex);

    void show_nametable_window();

    Emulator* emulator_;
    BUS* bus_;

    sf::Clock clock_;
    sf::Time lastUpdate_;
    sf::Time lastFPS_;
    uint64_t lastFrameCount_ = 0;
    uint64_t fps_ = 0;
    std::unique_ptr<sf::RenderWindow> window_;
    std::unique_ptr<sf::RenderWindow> namWindow_;
    sf::Font font_;
    byte_t pal_idx_ = 0;
    sf::Int64 step_rate_ = 1000;
};