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
    static sf::Texture debugTex;
    if (tex.getSize().x == 0)
    {
        tex.create(256, 240);
        debugTex.create(512, 480 + 128);
    }

    window_->clear();

    tex.update(ppu.output().data());
    sf::RectangleShape view({512.f, 480.f});
    view.setPosition(512.f, 0.f);
    view.setTexture(&tex);
    window_->draw(view);

    sf::Uint8 buffer[256 * 240 * 4];
    ppu.patterntable_img(buffer, 128*4, 0);
    debugTex.update(buffer, 128, 128, 0, 0);

    ppu.patterntable_img(buffer, 128*4, 1);
    debugTex.update(buffer, 128, 128, 128, 0);

    //ppu.nametable_img(buffer, 256 * 4, 0);
    //debugTex.update(buffer, 256, 240, 0, 128);
    
    //ppu.nametable_img(buffer, 256 * 4, 1);
    //debugTex.update(buffer, 256, 240, 256, 128);
    
    //ppu.nametable_img(buffer, 256 * 4, 2);
    //debugTex.update(buffer, 256, 240, 0, 128 + 240);
    
    //ppu.nametable_img(buffer, 256 * 4, 3);
    //debugTex.update(buffer, 256, 240, 256, 128 + 240);

    sf::Sprite pat(debugTex, {0, 0, 128 * 2, 128});
    pat.setScale(2, 2);
    pat.setPosition(0.f, window_->getSize().y - 256.f);
    window_->draw(pat);

    //sf::Sprite nam(debugTex, {0, 128, 512, 480});
    //nam.setPosition(0.f, 32.f + 128.f * 2);
    //window_->draw(nam);

    sf::Text text(sf::String("FRAME: ") += std::to_string(ppu.frame()), font_, 12);
    text.setFillColor(sf::Color::Green);
    window_->draw(text);
}
