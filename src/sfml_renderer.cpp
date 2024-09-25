#include "sfml_renderer.h"

#include "utils.h"

#include "emulator.h"
#include "platform/openfile_dialog.h"

#include "ui/global.h"
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
    ImGui::GetIO().IniFilename = "data/imgui.ini";
}

SFMLRenderer::~SFMLRenderer()
{
    ImGui::SFML::Shutdown();
}

bool SFMLRenderer::update()
{
    poll_events_();

    sf::Time t = clock_.getElapsedTime();
    sf::Time delta = t - last_update_;
    last_update_ = t;

    ImGui::SFML::Update(*window_, delta);

    if (window_->isOpen())
    {
        draw();

        ImGui::SFML::Render(*window_);

        window_->display();
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
}

void SFMLRenderer::draw()
{
    window_->clear(sf::Color(0x1e5dceff));

    PPU const& ppu = *emulator_->get_ppu();
    //CPU const& cpu = *emulator_->get_cpu();

    draw_game(ppu);

    ui::imgui_mainmenu();
    ui::imgui_debugwindows();
}

void SFMLRenderer::draw_game(PPU const& ppu)
{
    static GameViewport viewport;
    viewport.update(ppu);

    static sf::Vector2f offset{0.f, 16.f};
    sf::Vector2f viewSize{256.f, 240.f};

    constexpr bool resizable = true;

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