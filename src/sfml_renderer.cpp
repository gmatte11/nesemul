#include "sfml_renderer.h"

#include "utils.h"

#include "emulator.h"
#include "platform/openfile_dialog.h"

#include "ui/global.h"
#include "ui/debug_pages.h"
#include "ui/game_viewport.h"
#include "ui/imgui.h"

#include <imgui-SFML.h>

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
    std::wstring filepath;
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
    }
}

SFMLRenderer::SFMLRenderer(Emulator* emulator)
    : emulator_(emulator)
{
    window_.reset(new sf::RenderWindow(sf::VideoMode(1028, 720), "NESEMUL"));

    ImGui::SFML::Init(*window_);

    debug_step_.setPosition({8.f, 520.f});
    debug_cpu_.setPosition({520.f, 20.f});
    debug_ppu_.setPosition({520.f, 20.f});
    debug_pat_.setPosition({514.f, 720.f - 280.f});
}

SFMLRenderer::~SFMLRenderer()
{
    ImGui::SFML::Shutdown();
}

bool SFMLRenderer::update()
{
    poll_events_();

    sf::Time t = clock_.getElapsedTime();
    sf::Time delta = t - lastUpdate_;
    lastUpdate_ = t;

    ImGui::SFML::Update(*window_, delta);

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

        ImGui::SFML::Render(*window_);

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
        ImGui::SFML::ProcessEvent(*window_, ev);

        if (ev.type == sf::Event::Closed)
            window_->close();

        if (ev.type == sf::Event::Resized)
        {
            sf::FloatRect rect{ 0.f, 0.f, static_cast<float>(ev.size.width), static_cast<float>(ev.size.height) };
            window_->setView(sf::View(rect));
        }

        if (ev.type == sf::Event::KeyPressed)
        {
            switch (ev.key.code)
            {
            case sf::Keyboard::P: emulator_->toggle_pause(); break;
            case sf::Keyboard::O: if (ev.key.control) open_rom(*emulator_); break;

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

            debug_step_.on_event(ev);

            if (debug_page_ != 0)
                debug_pat_.on_event(ev);

            if (debug_page_ == 1)
                debug_cpu_.on_event(ev);

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
    //CPU const& cpu = *emulator_->get_cpu();

    draw_game(ppu);

    if (debug_page_ > 0)
    {
        debug_step_.update();
        window_->draw(debug_step_);

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

        std::string_view mode_str = emulator_->is_debugging() ? "(D)" : 
                                (emulator_->is_paused() ? "(P)" : "");
        auto sfmt = fmt::format("FRAME: {}  ({}%) {}", ppu.frame(), step_rate_ / 10, mode_str);
        text.setString(sfmt);

        window_->draw(text);
    }

    ui::imgui_mainmenu();
    ui::imgui_debugwindows();
}

void SFMLRenderer::draw_game(PPU const& ppu)
{
    static GameViewport viewport;
    viewport.update(ppu);

    static sf::Vector2f offset{0.f, 16.f};
    sf::Vector2f viewSize{256.f, 240.f};

    bool resizable = debug_page_ == 0;

    if (resizable)
    {
        sf::Vector2u winSizeu = window_->getSize();
        sf::Vector2f winSize{ static_cast<float>(winSizeu.x), static_cast<float>(winSizeu.y) };
        winSize -= offset;

        if (winSize.x < winSize.y)
        {
            viewSize.x = winSize.x;
            viewSize.y *= (winSize.x / viewSize.x);
        }
        else
        {
            viewSize.x *= (winSize.y / viewSize.y);
            viewSize.y = winSize.y;
        }
    }

    viewport.setSize(viewSize);
    viewport.setPosition(offset);
    window_->draw(viewport);
}

void SFMLRenderer::show_nametable_window()
{
    if (namWindow_ == nullptr || !namWindow_->isOpen())
    {
        namWindow_.reset(new sf::RenderWindow(sf::VideoMode(512, 480), "Nametables"));
    }
}