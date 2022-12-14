#include "debug_pages.h"

#include "debugger.h"
#include "emulator.h"

#include "ui/global.h"

#include <fmt/format.h>

#include <iterator>
#include <algorithm>
#include <fstream>

void PageBase::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (auto* d : drawables_)
        target.draw(*d, getTransform());
}


PageDebugStep::PageDebugStep()
{
    text_.setFont(ui::get_font());
    text_.setPosition({0.f, 0.f});
    text_.setCharacterSize(10);
    text_.setFillColor(sf::Color::White);

    drawables_ = { &text_ };
}

void PageDebugStep::update()
{
    Emulator* emulator = Emulator::instance();

    BUS* bus = emulator->get_bus();
    CPU const& cpu = *emulator->get_cpu();
    PPU const& ppu = *emulator->get_ppu();

    PPU_State const& ppu_state = ppu.get_state();
    address_t vram_addr = ppu.get_vram_addr();

    fmt::memory_buffer buf;
    fmt::format_to(buf, "VBL timing: {}\n", emulator->cpu_cycle_last_vblank);
    fmt::format_to(buf, "CPU cycles: {}   PPU cycles: {}\n", emulator->cpu_cycle_per_frame, emulator->ppu_cycle_per_frame);
    fmt::format_to(buf, "PPU: scanline {} cycle {} vblank {}\n", ppu_state.scanline_, ppu_state.cycle_, ppu_state.is_in_vblank_);
    
    struct VRAM
    {
        address_t X : 5;
        address_t Y : 5;
        address_t N : 2;
        address_t y : 3;
    } v;

    v = *reinterpret_cast<VRAM*>(&vram_addr);
    byte_t N = v.N;
    byte_t X = v.X;
    byte_t Y = v.Y;
    byte_t x = 0;
    byte_t y = v.y;

    fmt::format_to(buf, "VRAM N:{} X:{} Y:{} x:{} y:{}\n", N, X, Y, x, y);

    text_.setString(fmt::to_string(buf));
}

void PageDebugStep::on_event(sf::Event& ev)
{
    Debugger* debugger = Debugger::instance();

    switch (ev.key.code)
    {
    case sf::Keyboard::O: if (!ev.key.shift) debugger->request_break(Debugger::MODE_CPU_FETCH); break;
    case sf::Keyboard::I: debugger->request_break(Debugger::MODE_PPU_FRAME); break;
    case sf::Keyboard::L: if (!ev.key.shift) debugger->request_break(Debugger::MODE_PPU_LINE); break;
    }
}

PageDebugCPU::PageDebugCPU()
{
    text_.setFont(ui::get_font());
    text_.setPosition({0.f, 0.f});
    text_.setCharacterSize(10);
    text_.setFillColor(sf::Color::White);

    drawables_ = { &text_ };
}

void PageDebugCPU::update()
{
    Emulator* emulator = Emulator::instance();

    BUS* bus = emulator->get_bus();
    CPU const& cpu = *emulator->get_cpu();

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
        emulator->disassembler_.render(buf, cpu_state.program_counter_, offset);
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

    text_.setString(fmt::to_string(buf));
}

void PageDebugCPU::on_event(sf::Event& ev)
{
    Emulator* emulator = Emulator::instance();

    auto flush_report = [=]
    {
        std::string s = emulator->get_cpu()->get_state().stats_.report();
        
        std::ofstream out("callstats_report.txt");
        if (out.is_open())
        {
            out.write(s.data(), s.length());
        }
    };

    switch (ev.key.code)
    {
    case sf::Keyboard::Q: if (ev.key.shift) flush_report();
    }
}

PageDebugPPU::PageDebugPPU()
{
    // layout 2 sprite wide, 32 long
    texture_.create(8 * 2, 8 * 32);

    for (int i = 0; i < 64; i += 2)
    {
        const int row = i / 2;

        sf::Sprite& left = oam_sprites_[i];
        sf::Sprite& right = oam_sprites_[i + 1];

        left.setTexture(texture_);
        left.setTextureRect({ 0, 8 * row, 8, 8 });
        left.setPosition({0.f, 13.f * row });
        left.setScale({1.5f, 1.5f});

        right.setTexture(texture_);
        right.setTextureRect({ 8, 8 * row, 8, 8 });
        right.setPosition({210.f, 13.f * row });
        right.setScale({1.5f, 1.5f});
    }

    oam_text_.setPosition({20.f, 0.f});
    oam_text_.setFont(ui::get_font());
    oam_text_.setCharacterSize(10);
    oam_text_.setFillColor(sf::Color::White);

    drawables_.resize(oam_sprites_.size() + 1);
    drawables_[0] = &oam_text_;
    std::transform(oam_sprites_.begin(), oam_sprites_.end(), &drawables_[1], [](auto& sprite) { return &sprite; });
}

void PageDebugPPU::update()
{
    PPU const& ppu = *Emulator::instance()->get_ppu();

    fmt::memory_buffer buffer;
    Image<8, 8> tile_buf;

    for (int i = 0; i < 32; ++i)
    {
        const int row = i;

        struct Sprite
        {
            byte_t y_;
            byte_t tile_;
            byte_t att_;
            byte_t x_;
        };

        Sprite const& s1 = reinterpret_cast<const Sprite*>(ppu.oam_data())[i];
        Sprite const& s2 = reinterpret_cast<const Sprite*>(ppu.oam_data())[i + 32];

        ppu.tile_img(tile_buf, s1.tile_, ppu.ppuctrl_.fg_pat_, ppu.get_palette((s1.att_ & 0x3) + 4));
        texture_.update(tile_buf.data(), 8, 8, 0, 8 * row);

        ppu.tile_img(tile_buf, s2.tile_, ppu.ppuctrl_.fg_pat_, ppu.get_palette((s1.att_ & 0x3) + 4));
        texture_.update(tile_buf.data(), 8, 8, 8, 8 * row);

        fmt::format_to(buffer, "{:02x} ({:3}, {:3}), {:02x}\t{:02x} ({:3}, {:3}), {:02x}\n", s1.tile_, s1.x_, s1.y_, s1.att_, s2.tile_, s2.x_, s2.y_, s2.att_);
    }

    oam_text_.setString(fmt::to_string(buffer));
}

PageDebugPAT::PageDebugPAT()
{
    pal_texture_.create(302, 12);
    pat_texture_.create(256, 128);

    pal_sprite_.setTexture(pal_texture_, true);
    pal_sprite_.setPosition({6.f, 0.f});
    pal_sprite_.setScale({1.5f, 1.5f});

    pat_sprite_.setTexture(pat_texture_, true);
    pat_sprite_.setPosition({20.f, 24.f});
    pat_sprite_.setScale({1.5f, 1.5f});
    
    selector_.setSize({48.f, 12.f});
    selector_.setFillColor(sf::Color::Transparent);
    selector_.setOutlineColor(sf::Color::White);
    selector_.setOutlineThickness(3.f);

    drawables_ = { &pal_sprite_, &pat_sprite_, &selector_ };
}

void PageDebugPAT::update()
{
    PPU const& ppu = *Emulator::instance()->get_ppu();

    pal_update_(ppu);
    pat_update_(ppu);

    selector_.setPosition({9.f + pal_idx_ * 57.f, 3.f});
}

void PageDebugPAT::pal_update_(PPU const& ppu)
{
    sf::Texture& tex = pal_texture_;

    sf::Image buffer;
    buffer.create(tex.getSize().x, tex.getSize().y, sf::Color::Transparent);
    tex.update(buffer, 0, 0);

    for (byte_t i = 0; i < 8; ++i)
    {
        Palette p = ppu.get_palette(i);

        for (int j = 0; j < 4; ++j)
        {
            Color const& c = p.get(j);

            sf::Color sfc(c.r, c.g, c.b);
            buffer.create(8, 8, sfc);

            int x = (i * (4 * 8 + 6)) + j * 8 + 2;
            tex.update(buffer, x, 2);
        }
    }
}

void PageDebugPAT::pat_update_(PPU const& ppu)
{
    Image<128, 128> buffer;
    ppu.patterntable_img(buffer, 0, ppu.get_palette(pal_idx_));
    pat_texture_.update(buffer.data(), 128, 128, 0, 0);

    ppu.patterntable_img(buffer, 1, ppu.get_palette(pal_idx_));
    pat_texture_.update(buffer.data(), 128, 128, 128, 0);
}

void PageDebugPAT::on_event(sf::Event& ev)
{
    if (ev.type == sf::Event::KeyPressed)
    {
        if (ev.key.code == sf::Keyboard::LBracket)
            pal_idx_ = (8 + (pal_idx_ - 1)) % 8;
        else if (ev.key.code == sf::Keyboard::RBracket)
            pal_idx_ = (pal_idx_ + 1) % 8;
    }
}