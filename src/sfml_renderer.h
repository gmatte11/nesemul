#pragma once

#include "types.h"
#include <SFML/Graphics.hpp>

namespace sf
{
    class RenderWindow;
}

class PPU;
class BUS;

class SFMLRenderer
{
public:
    SFMLRenderer(BUS* bus);
    ~SFMLRenderer();
    
    bool update();
    bool timeout();

    bool toggle_pause() { pause_ = !pause_; return pause_; }
    bool is_paused() const { return pause_; }
    bool execute_frame() 
    {  
        bool ret = stepFrame_;
        stepFrame_ = false;
        return ret;
    }

private:
    void draw();
    void draw_game(PPU const& ppu);
    void draw_pat(PPU const& ppu);
    void draw_pal(PPU const& ppu);
    void draw_oam(PPU const& ppu);
    void draw_nam(PPU const& ppu);

    BUS* bus_;

    sf::Clock clock_;
    sf::Time lastUpdate_;
    sf::Time lastFPS_;
    uint64_t lastFrameCount_ = 0;
    uint64_t fps_ = 0;
    std::unique_ptr<sf::RenderWindow> window_;
    sf::Font font_;
    byte_t pal_idx_ = 0;
    bool pause_ = false;
    bool stepFrame_ = false;
    sf::Int64 stepRate_ = 1000;
};