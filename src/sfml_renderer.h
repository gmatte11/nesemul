#pragma once

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
    
    bool update(const PPU& ppu);
    bool timeout();

    bool toggle_pause() { pause_ = !pause_; return pause_; }
    bool is_paused() const { return pause_; }

private:
    void draw(const PPU& ppu);

    BUS* bus_;

    sf::Clock clock_;
    sf::Time lastUpdate_;
    sf::Time lastFPS_;
    uint64_t lastFrameCount_ = 0;
    uint64_t fps_ = 0;
    std::unique_ptr<sf::RenderWindow> window_;
    sf::Font font_;
    bool pause_ = false;
};