#pragma once

#include <SFML/Graphics.hpp>

namespace sf
{
    class RenderWindow;
}

class PPU;

class SFMLRenderer
{
public:
    SFMLRenderer();
    ~SFMLRenderer();
    
    bool update(const PPU& ppu);
    bool timeout();

private:
    void draw(const PPU& ppu);

    sf::Clock clock_;
    sf::Time lastUpdate_;
    sf::Time lastFPS_;
    int lastFrameCount_ = 0;
    int fps_ = 0;
    std::unique_ptr<sf::RenderWindow> window_;
    sf::Font font_;
};