#include "sfml_renderer.h"

#include "emulator.h"
#include "platform/openfile_dialog.h"

#include "ui/global.h"
#include "ui/debug_pages.h"
#include "ui/game_viewport.h"

#include <fmt/format.h>

#include <iostream>

struct Map { sf::Keyboard::Key sfmlKey; Controller::Button button; };
static std::array<Map, 8> g_mapping = {
    Map{sf::Keyboard::S, Controller::A}, {sf::Keyboard::D, Controller::B},
    {sf::Keyboard::Enter, Controller::Start}, {sf::Keyboard::Backspace, Controller::Select},
    {sf::Keyboard::Up, Controller::Up}, {sf::Keyboard::Down, Controller::Down},
    {sf::Keyboard::Left, Controller::Left}, {sf::Keyboard::Right, Controller::Right}
};

static void open_rom(Emulator& emulator)
{
    std::string filepath;
    if (openfile_dialog(filepath))
    {
        try
        {
            emulator.read_rom(filepath);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        emulator.reset();
    }
}

SFMLRenderer::SFMLRenderer(Emulator* emulator)
    : emulator_(emulator)
{
    window_.reset(new sf::RenderWindow(sf::VideoMode(1028, 720), "NESEMUL"));

    debug_cpu_.setPosition({520.f, 20.f});
    debug_ppu_.setPosition({520.f, 20.f});
    debug_pat_.setPosition({514.f, 720.f - 280.f});
}

SFMLRenderer::~SFMLRenderer()
{}

bool SFMLRenderer::update()
{
    poll_events_(); 

    sf::Time t = clock_.getElapsedTime();
    if ((t - lastFPS_).asSeconds() >= 1)
    {
        PPU const& ppu = *emulator_->get_ppu();
        fps_ = ppu.frame() - lastFrameCount_;
        lastFrameCount_ = ppu.frame();
        lastFPS_ = t;
    }

    if (window_->isOpen())
    {
        sf::String title = fmt::format("NESEMUL (FPS: {})", fps_);
        window_->setTitle(title);
        draw();
        window_->display();
    }

    if (namWindow_ && namWindow_->isOpen())
    {
        static NametableViewport viewport;
        viewport.update(*emulator_->get_ppu());
        namWindow_->draw(viewport);

        namWindow_->display();
    }

    return window_->isOpen();
}

void SFMLRenderer::poll_events_()
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
            case sf::Keyboard::P: emulator_->toggle_pause(); break;
            case sf::Keyboard::O: if (ev.key.shift) open_rom(*emulator_); break;

            case sf::Keyboard::Hyphen: step_rate_ = std::max(step_rate_ - 100, 100ll); break;
            case sf::Keyboard::Equal: step_rate_ = std::min(step_rate_ + 100, 2000ll); break;
            case sf::Keyboard::Num0: step_rate_ = 100; break;

            case sf::Keyboard::N: if (ev.key.shift) show_nametable_window(); break;

            case sf::Keyboard::K: debug_page_ = (debug_page_ + 1) % 3; break;

            case sf::Keyboard::R: if (ev.key.shift) emulator_->reset(); break;

            default: break;
            }

            for (Map m : g_mapping)
            {
                if (ev.key.code == m.sfmlKey)
                {
                    emulator_->press_button(m.button);
                    break;
                }
            }

            switch(debug_page_)
            {
                case 1:
                    debug_cpu_.on_event(ev);
                    debug_pat_.on_event(ev);
                    break;
                
                case 2: 
                    debug_ppu_.on_event(ev);
                    debug_pat_.on_event(ev);
                    break;
            }
        }

        if (ev.type == sf::Event::KeyReleased)
        {
            for (Map m : g_mapping)
            {
                if (ev.key.code == m.sfmlKey)
                {
                    emulator_->release_button(m.button);
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
}

void SFMLRenderer::draw()
{
    window_->clear(sf::Color(0x1e5dceff));

    PPU const& ppu = *emulator_->get_ppu();
    CPU const& cpu = *emulator_->get_cpu();

    draw_game(ppu);

    if (debug_page_ > 0)
    {
        debug_pat_.update();
        window_->draw(debug_pat_);

        switch (debug_page_)
        {
        case 1:
            debug_cpu_.update();
            window_->draw(debug_cpu_);
            break;
        case 2:
            debug_ppu_.update();
            window_->draw(debug_ppu_);
            break;
        }

        static sf::String empty;
        sf::Text text(empty, ui::get_font(), 12);
        text.setPosition({520.f, 0.f});
        text.setFillColor(sf::Color::White);

        auto sfmt = fmt::format("FRAME: {}  ({}%) {}", ppu.frame(), step_rate_ / 10, (emulator_->is_paused()) ? "(P)" : "");
        text.setString(sfmt);

        window_->draw(text);
    }
}

void SFMLRenderer::draw_game(PPU const& ppu)
{
    static GameViewport viewport;
    viewport.update(ppu);

    sf::Vector2f viewSize = (debug_page_ == 0) ? sf::Vector2f{768.f, 720.f} : sf::Vector2f{512.f, 480.f};
    viewport.setSize(viewSize);

    viewport.setPosition({0.f, 31.f});
    window_->draw(viewport);
}

void SFMLRenderer::show_nametable_window()
{
    if (namWindow_ == nullptr || !namWindow_->isOpen())
    {
        namWindow_.reset(new sf::RenderWindow(sf::VideoMode(512, 480), "Nametables"));
    }
}