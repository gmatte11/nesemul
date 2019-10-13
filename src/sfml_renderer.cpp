#include "sfml_renderer.h"

#include "bus.h"
#include "ppu.h"
#include "SFML/Graphics.hpp"

struct Map { sf::Keyboard::Key sfmlKey; Controller::Button button; };
static std::array<Map, 8> g_mapping = {
    Map{sf::Keyboard::S, Controller::A}, {sf::Keyboard::D, Controller::B},
    {sf::Keyboard::Enter, Controller::Start}, {sf::Keyboard::Backspace, Controller::Select},
    {sf::Keyboard::Up, Controller::Up}, {sf::Keyboard::Down, Controller::Down},
    {sf::Keyboard::Left, Controller::Left}, {sf::Keyboard::Right, Controller::Right}
};

SFMLRenderer::SFMLRenderer(BUS* bus)
    : bus_(bus)
{
    window_.reset(new sf::RenderWindow(sf::VideoMode(1024, 720), "NESEMUL"));
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

        if (ev.type == sf::Event::KeyPressed)
        {
            if (ev.key.code == sf::Keyboard::P)
            {
                toggle_pause();
            }

            for (Map m : g_mapping)
            {
                if (ev.key.code == m.sfmlKey)
                {
                    bus_->ctrl_.press(m.button);
                    break;
                }
            }
        }

        if (ev.type == sf::Event::KeyReleased)
        {
            for (Map m : g_mapping)
            {
                if (ev.key.code == m.sfmlKey)
                {
                    bus_->ctrl_.release(m.button);
                    break;
                }
            }
        }
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

    return window_->isOpen();
}

bool SFMLRenderer::timeout()
{
    sf::Time t = clock_.getElapsedTime();
    bool tick = (t - lastUpdate_) >= sf::microseconds(16'667);
    if (tick) lastUpdate_ = t;
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

    Image<128, 128> buffer;
    ppu.patterntable_img(buffer, 0);
    debugTex.update(buffer.data(), 128, 128, 0, 0);

    ppu.patterntable_img(buffer, 1);
    debugTex.update(buffer.data(), 128, 128, 128, 0);

    //ppu.nametable_img(buffer, 256 * 4, 0);
    //debugTex.update(buffer, 256, 240, 0, 128);
    
    //ppu.nametable_img(buffer, 256 * 4, 1);
    //debugTex.update(buffer, 256, 240, 256, 128);
    
    //ppu.nametable_img(buffer, 256 * 4, 2);
    //debugTex.update(buffer, 256, 240, 0, 128 + 240);
    
    //ppu.nametable_img(buffer, 256 * 4, 3);
    //debugTex.update(buffer, 256, 240, 256, 128 + 240);

    sf::Sprite pat(debugTex, {0, 0, 128 * 2, 128});
    pat.setScale(2.f, 2.f);
    pat.setPosition(0.f, 480.f - 256.f);
    window_->draw(pat);

    //sf::Sprite nam(debugTex, {0, 128, 512, 480});
    //nam.setPosition(0.f, 32.f + 128.f * 2);
    //window_->draw(nam);

    static sf::String empty;
    sf::Text text(empty, font_, 12);
    text.setFillColor(sf::Color::Green);
    if (!is_paused())
        text.setString(sf::String("FRAME: ") += std::to_string(ppu.frame()));
    else
        text.setString("PAUSED");
    window_->draw(text);

    sf::RectangleShape debug_bg({1024.f, 240.f});
    debug_bg.setFillColor(sf::Color(0x1e5dceff));
    debug_bg.setPosition({0.f, 480.f});
    window_->draw(debug_bg);

    CPU& cpu_ = bus_->cpu_;
    size_t idx = (cpu_.log_idx_ + cpu_.log_ring_.size() - 14) % cpu_.log_ring_.size();

    sf::Vector2f pos(20.f, 470.f);
    sf::Text line(empty, font_, 12);
    line.setFillColor(sf::Color::White);

    for (int i = 0; i < 14; ++i)
    {
        pos.y += 16.f;
        line.setString(cpu_.log_ring_[idx].data());
        line.setPosition(pos);
        window_->draw(line);

        (idx += 1) %= cpu_.log_ring_.size();
    }
    
    pos.x -= 16.f;
    line.setPosition(pos);
    line.setString("*");
    window_->draw(line);
    
}
