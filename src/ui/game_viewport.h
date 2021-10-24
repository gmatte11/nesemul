#pragma once

#include <SFML/Graphics.hpp>

class PPU;

class GameViewport : public sf::RectangleShape
{
public:
    GameViewport();
    void update(PPU const& ppu);

private:
    sf::Texture texture_;
};

class NametableViewport : public sf::RectangleShape
{
public:
    NametableViewport();
    void update(PPU const& ppu);

private:
    sf::Texture texture_;
};