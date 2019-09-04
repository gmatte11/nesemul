#include "sfml_renderer.h"

#include "ppu.h"
#include "SFML/Graphics.hpp"

SFMLRenderer::SFMLRenderer()
{
    window_.reset(new sf::RenderWindow(sf::VideoMode(1024, 480), "NESEMUL"));
    font_.loadFromFile("data/Emulogic-zrEw.ttf");
}

SFMLRenderer::~SFMLRenderer()
{}

bool SFMLRenderer::update(PPU const& ppu)
{
    sf::Event ev;
    while (window_->pollEvent(ev))
    {
        if (ev.type == sf::Event::Closed)
            window_->close();
    }

    sf::Time t = clock_.getElapsedTime();
    if ((t - lastFPS_).asSeconds() >= 1)
    {
        fps_ = ppu.frame() - lastFrameCount_;
        lastFrameCount_ = ppu.frame();
        lastFPS_ = t;
    }

    if (window_->isOpen())
    {
        window_->setTitle(sf::String("NESEMUL (FPS: ") += sf::String(std::to_string(fps_)) += ")");
        draw(ppu);
        window_->display();
    }

    lastUpdate_  = t;
    return window_->isOpen();
}

bool SFMLRenderer::timeout()
{
    sf::Time t = clock_.getElapsedTime();
    bool tick = (t - lastUpdate_).asMilliseconds() >= 14;
    return tick;
}

void SFMLRenderer::draw(PPU const& ppu)
{
    static sf::Texture tex;
    if (tex.getSize().x == 0)
        tex.create(256, 240);

    window_->clear();

    tex.update(ppu.output().data(), 256, 240, 0, 0);
    sf::RectangleShape shape(sf::Vector2f(256.f * 2, 240.f * 2));
    shape.setTexture(&tex);
    shape.setPosition(512.f, 0.f);
    window_->draw(shape);

    sf::Text text(sf::String("FRAME: ") += std::to_string(ppu.frame()), font_, 24);
    text.setFillColor(sf::Color::Green);
    text.scale(.5f, .5f);
    window_->draw(text);
}
