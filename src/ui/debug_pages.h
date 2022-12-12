#pragma once

#include "types.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <memory>
#include <vector>

class CPU;
class PPU;

class PageBase : public sf::Drawable, public sf::Transformable
{
public:
    PageBase() = default;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

protected:
    std::vector<sf::Drawable*> drawables_;
};

class PageDebugStep : public PageBase
{
public:
    PageDebugStep();

    void update();
    void on_event(sf::Event& ev);

private:
    sf::Text text_;
};

class PageDebugCPU : public PageBase
{
public:
    PageDebugCPU();

    void update();
    void on_event(sf::Event& ev);

private:
    sf::Text text_;
};

class PageDebugPPU : public PageBase
{
public:
    PageDebugPPU();

    void update();

private:
    sf::Text oam_text_;
    std::array<sf::Sprite, 64> oam_sprites_;
    sf::Texture texture_;
};

class PageDebugPAT : public PageBase
{
public:
    PageDebugPAT();

    void update();
    void on_event(sf::Event& ev);

private:
    void pal_update_(PPU const& ppu);
    void pat_update_(PPU const& ppu);

    byte_t pal_idx_ = 0;

    sf::RectangleShape selector_;
    sf::Sprite pal_sprite_;
    sf::Sprite pat_sprite_;

    sf::Texture pal_texture_;
    sf::Texture pat_texture_;
};
