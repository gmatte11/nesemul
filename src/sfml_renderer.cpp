#include "sfml_renderer.h"

#include "emulator.h"

#include "ui/global.h"

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
{
    window_.reset(new sf::RenderWindow(sf::VideoMode(1028, 720), "NESEMUL"));

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
            case sf::Keyboard::P: emulator_->toggle_pause(); break;
            case sf::Keyboard::O: emulator_->step_once(); break;
            case sf::Keyboard::I: emulator_->step_frame(); break;

            case sf::Keyboard::Hyphen: step_rate_ = std::max(step_rate_ - 100, 100ll); break;
            case sf::Keyboard::Equal: step_rate_ = std::min(step_rate_ + 100, 2000ll); break;
            case sf::Keyboard::Num0: step_rate_ = 100; break;

            case sf::Keyboard::N: if (ev.key.shift) show_nametable_window(); break;

            case sf::Keyboard::L: pal_idx_ = (pal_idx_ + 1) % 8; break;

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

void SFMLRenderer::draw()
{
    window_->clear(sf::Color(0x1e5dceff));

    PPU const& ppu = *emulator_->get_ppu();
    CPU const& cpu = *emulator_->get_cpu();

    draw_game(ppu);

    if (debug_page_ > 0)
    {
        draw_pal(ppu);
        draw_pat(ppu);

        switch (debug_page_)
        {
        case 1:
            draw_asm(cpu);
            break;
        case 2:
            draw_oam(ppu);
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
}

void SFMLRenderer::draw_game(PPU const& ppu)
{
    static sf::Texture tex;
    if (tex.getSize().x == 0)
    {
        tex.create(256, 240);
    }
    
    tex.update(ppu.output().data());

    sf::Vector2f viewSize = (debug_page_ == 0) ? sf::Vector2f{768.f, 720.f} : sf::Vector2f{512.f, 480.f};

    sf::RectangleShape view(viewSize);
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
    Image<8, 8> tile_buf;
    static sf::Texture tex;
    if (tex.getSize().x == 0)
    {
        tex.create(8, 8);
    }

    sf::RectangleShape tile({8.f, 8.f});
    tile.setTexture(&tex, true);
    tile.setScale({1.5f, 1.5f});

    for (int i = 0; i < 32; ++i)
    {
        struct Sprite
        {
            byte_t y_;
            byte_t tile_;
            byte_t att_;
            byte_t x_;
        };

        Sprite const& s1 = reinterpret_cast<const Sprite*>(ppu.oam_.data())[i];
        Sprite const& s2 = reinterpret_cast<const Sprite*>(ppu.oam_.data())[i + 32];

        fmt::format_to(buffer, "{:02x} ({:3}, {:3}), {:02x}\t{:02x} ({:3}, {:3}), {:02x}", s1.tile_, s1.x_, s1.y_, s1.att_, s2.tile_, s2.x_, s2.y_, s2.att_);

        if (i < 31)
            buffer.push_back('\n');
        
        ppu.tile_img(tile_buf, s1.tile_, ppu.ppuctrl_.fg_pat_, ppu.get_palette((s1.att_ & 0x3) + 4));
        tex.update(tile_buf.data());
        tile.setPosition({520.f, 20.f + 13.f * i});
        window_->draw(tile);

        ppu.tile_img(tile_buf, s2.tile_, ppu.ppuctrl_.fg_pat_, ppu.get_palette((s2.att_ & 0x3) + 4));
        tex.update(tile_buf.data());
        tile.setPosition({730.f, 20.f + 13.f * i});
        window_->draw(tile);
    }

    sf::Text oam(fmt::to_string(buffer), ui::get_font(), 10);
    oam.setPosition({540.f, 20.f});
    oam.setFillColor(sf::Color::White);
    window_->draw(oam);
}

void SFMLRenderer::draw_asm(CPU const& cpu)
{
    BUS* bus = emulator_->get_bus();

    fmt::memory_buffer buf;

    CPU_State const& cpu_state = cpu.get_state();
    {
        fmt::format_to(buf, "Flags: {}{}xx{}{}{}{}\n"
            , (cpu_state.status_ & CPU_State::kNegative) ? 'N' : '-'
            , (cpu_state.status_ & CPU_State::kOverflow) ? 'O' : '-'
            , (cpu_state.status_ & CPU_State::kDecimal) ? 'D' : '-'
            , (cpu_state.status_ & CPU_State::kIntDisable) ? 'I' : '-'
            , (cpu_state.status_ & CPU_State::kZero) ? 'Z' : '-'
            , (cpu_state.status_ & CPU_State::kCarry) ? 'C' : '-'
            );

        fmt::format_to(buf, "A:{:02x} X:{:02x} Y:{:02x} SP:{:02x}\n"
            , cpu_state.accumulator_
            , cpu_state.register_x_
            , cpu_state.register_y_
            , cpu_state.stack_pointer_);
    }

    for (int offset = -5; offset <= 5; ++offset)
    {
        buf.push_back('\n');
        emulator_->disassembler_.render(buf, cpu_state.program_counter_, offset);
    }

    // Some tests write status value to $6000
    fmt::format_to(buf, "\n\n$6000: {:02x}", bus->read_cpu(0x6000));
    fmt::format_to(buf, "\n$6001: {:02x}", bus->read_cpu(0x6001));
    fmt::format_to(buf, "\n$6002: {:02x}", bus->read_cpu(0x6002));
    fmt::format_to(buf, "\n$6003: {:02x}", bus->read_cpu(0x6003));

    fmt::format_to(buf, "\n\n$6004: ");

    {
        address_t it = 0x6004;
        byte_t c;
        for (;;)
        {
            c = bus->read_cpu(it);
            if (c == 0)
                break;

            ++it;
            buf.push_back((char)c);
        }
    }

    sf::Text text(buf.data(), ui::get_font(), 10);
    text.setPosition({520.f, 51.f});
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