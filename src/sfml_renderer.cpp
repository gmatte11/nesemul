#include "sfml_renderer.h"

#include "emulator.h"
#include "SFML/Graphics.hpp"

#include <fmt/format.h>

struct Map { sf::Keyboard::Key sfmlKey; Controller::Button button; };
static std::array<Map, 8> g_mapping = {
    Map{sf::Keyboard::S, Controller::A}, {sf::Keyboard::D, Controller::B},
    {sf::Keyboard::Enter, Controller::Start}, {sf::Keyboard::Backspace, Controller::Select},
    {sf::Keyboard::Up, Controller::Up}, {sf::Keyboard::Down, Controller::Down},
    {sf::Keyboard::Left, Controller::Left}, {sf::Keyboard::Right, Controller::Right}
};

SFMLRenderer::SFMLRenderer(Emulator* emulator)
    : emulator_(emulator)
    , bus_(emulator->get_bus())
{
    window_.reset(new sf::RenderWindow(sf::VideoMode(1028, 720), "NESEMUL"));
    font_.loadFromFile("data/Emulogic-zrEw.ttf");
}

SFMLRenderer::~SFMLRenderer()
{}

bool SFMLRenderer::update()
{
    sf::Event ev;
    while (window_->pollEvent(ev))
    {
        if (ev.type == sf::Event::Closed)
            window_->close();

        if (ev.type == sf::Event::KeyPressed)
        {
            switch (ev.key.code)
            {
            case sf::Keyboard::P: toggle_pause(); break;
            case sf::Keyboard::O: if (is_paused()) stepFrame_ = true; break;

            case sf::Keyboard::Hyphen: stepRate_ = std::max(stepRate_ - 100, 100ll); break;
            case sf::Keyboard::Equal: stepRate_ = std::min(stepRate_ + 100, 2000ll); break;
            case sf::Keyboard::Num0: stepRate_ = 100; break;

            case sf::Keyboard::N: if (ev.key.shift) show_nametable_window(); break;

            case sf::Keyboard::I: pal_idx_ = (pal_idx_ + 1) % 8; break;

            default: break;
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

    while (namWindow_ && namWindow_->pollEvent(ev))
    {
        if (ev.type == sf::Event::Closed)
            namWindow_->close();
    }

    sf::Time t = clock_.getElapsedTime();
    if ((t - lastFPS_).asSeconds() >= 1)
    {
        PPU const& ppu = bus_->ppu_;
        fps_ = ppu.frame() - lastFrameCount_;
        lastFrameCount_ = ppu.frame();
        lastFPS_ = t;
    }

    if (window_->isOpen())
    {
        window_->setTitle(sf::String("NESEMUL (FPS: ") += sf::String(std::to_string(fps_)) += ")");
        draw();
        window_->display();
    }

    if (namWindow_ && namWindow_->isOpen())
    {
        namWindow_->display();
    }

    return window_->isOpen();
}

bool SFMLRenderer::timeout()
{
    static constexpr sf::Int64 rate = 16'667;

    sf::Time t = clock_.getElapsedTime();
    bool tick = (t - lastUpdate_) >= sf::microseconds(rate * 1000 / stepRate_);
    if (tick) lastUpdate_ = t;
    return tick;
}

void SFMLRenderer::draw()
{
    window_->clear(sf::Color(0x1e5dceff));

    PPU const& ppu = bus_->ppu_;
    CPU const& cpu = bus_->cpu_;


    draw_game(ppu);
    draw_pal(ppu);
    draw_pat(ppu);
    //draw_oam(ppu);
    draw_asm(cpu);

    static sf::String empty;
    sf::Text text(empty, font_, 12);
    text.setPosition({520.f, 0.f});
    text.setFillColor(sf::Color::White);

    auto sfmt = fmt::format("FRAME: {}  ({}%) {}", ppu.frame(), stepRate_ / 10, (is_paused()) ? "(P)" : "");
    text.setString(sfmt);
    
    window_->draw(text);

    if (namWindow_ && namWindow_->isOpen())
    {
        static sf::Texture tex;
        if (tex.getSize().x == 0)
        {
            tex.create(512, 480);
        }

        draw_nam(ppu, &tex);

        sf::RectangleShape shape({512.f, 480.f});
        shape.setTexture(&tex, true);
        namWindow_->draw(shape);
    }

    /*CPU& cpu_ = bus_->cpu_;
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
    window_->draw(line);*/
    
}

void SFMLRenderer::draw_game(PPU const& ppu)
{
    static sf::Texture tex;
    if (tex.getSize().x == 0)
    {
        tex.create(256, 240);
    }
    
    tex.update(ppu.output().data());
    sf::RectangleShape view({512.f, 480.f});
    view.setPosition(0.f, 0.f);
    view.setTexture(&tex);
    window_->draw(view);
}

void SFMLRenderer::draw_pat(PPU const& ppu)
{
    static sf::Texture tex;
    if (tex.getSize().x == 0)
    {
        tex.create(256, 128);
    }

    Image<128, 128> buffer;
    ppu.patterntable_img(buffer, 0, ppu.get_palette(pal_idx_));
    tex.update(buffer.data(), 128, 128, 0, 0);

    ppu.patterntable_img(buffer, 1, ppu.get_palette(pal_idx_));
    tex.update(buffer.data(), 128, 128, 128, 0);

    sf::RectangleShape pat({512.f, 256.f});
    pat.setTexture(&tex, true);
    pat.setPosition({514.f, 720.f - 258.f});
    window_->draw(pat);
}

void SFMLRenderer::draw_pal(PPU const& ppu)
{
    static sf::Texture tex;
    if (tex.getSize().x == 0)
    {
        tex.create(302, 12);
    }

    sf::Image buffer;
    buffer.create(tex.getSize().x, tex.getSize().y, sf::Color::Transparent);
    tex.update(buffer, 0, 0);

    for (int i = 0; i < 8; ++i)
    {
        Palette p = ppu.get_palette(i);

        if (i == pal_idx_)
        {
            buffer.create(36, 12, sf::Color::White);

            int x = i * (4 * 8 + 6);
            tex.update(buffer, x, 0);
        }

        for (int j = 0; j < 4; ++j)
        {
            Color const& c = p.get(j);

            sf::Color sfc(c.r, c.g, c.b);
            buffer.create(8, 8, sfc);

            int x = (i * (4 * 8 + 6)) + j * 8 + 2;
            tex.update(buffer, x, 2);
        }
    }

    constexpr float x = 516.f;
    constexpr float y = 720.f - 280.f;

    sf::RectangleShape pal({302.f, 12.f});
    pal.setTexture(&tex, true);
    pal.setPosition({x, y});
    pal.setScale({1.5f, 1.5f});
    window_->draw(pal);
}

void SFMLRenderer::draw_oam(PPU const& ppu)
{
    fmt::memory_buffer buffer;

    for (int i = 0; i < 32; ++i)
    {
        Sprite const& s1 = reinterpret_cast<const Sprite*>(ppu.oam_.data())[i];
        Sprite const& s2 = reinterpret_cast<const Sprite*>(ppu.oam_.data())[i + 32];

        fmt::format_to(buffer, "{:02x} ({:3}, {:3}), {:02x}\t{:02x} ({:3}, {:3}), {:02x}", s1.tile_, s1.x_, s1.y_, s1.att_, s2.tile_, s2.x_, s2.y_, s2.att_);

        if (i < 31)
            buffer.push_back('\n');
    }

    sf::Text oam(fmt::to_string(buffer), font_, 10);
    oam.setPosition({520.f, 20.f});
    oam.setFillColor(sf::Color::White);
    window_->draw(oam);
}

void SFMLRenderer::draw_asm(CPU const& cpu)
{
    fmt::memory_buffer buf;

    for (int offset = -5; offset < 5; ++offset)
    {
        if (buf.size() > 0)
            buf.push_back('\n');
        
        emulator_->disassembler_.render(buf, cpu.get_program_counter(), offset);
    }

    sf::Text text(buf.data(), font_, 10);
    text.setPosition({520.f, 20.f});
    text.setFillColor(sf::Color::White);
    window_->draw(text);
}

void SFMLRenderer::draw_nam(PPU const& ppu, sf::Texture* tex)
{
    if (tex->getSize().x < 512 || tex->getSize().y < 480)
        return;

    Image<256, 240> image;
    for (int i = 0; i < 4; ++i)
    {
        ppu.nametable_img(image, i);

        int x = (i % 2 == 0) ? 0 : 256;
        int y = (i / 2 == 0) ? 0 : 240;
        tex->update(image.data(), 256, 240, x, y);
    }
}

void SFMLRenderer::show_nametable_window()
{
    if (namWindow_ == nullptr || !namWindow_->isOpen())
    {
        namWindow_.reset(new sf::RenderWindow(sf::VideoMode(512, 480), "Nametables"));
    }
}