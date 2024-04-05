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

private:
    void poll_events_();

    void draw();
    void draw_game(PPU const& ppu);

    void show_nametable_window();

    Emulator* emulator_;

    sf::Clock clock_;
    sf::Time lastUpdate_;
    sf::Time lastFPS_;
    uint64_t lastFrameCount_ = 0;
    uint64_t fps_ = 0;
    std::unique_ptr<sf::RenderWindow> window_;
    sf::Int64 step_rate_ = 1000;
};