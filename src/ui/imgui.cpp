#include "imgui.h"

#include "emulator.h"
#include "utils.h"

#include "platform/platform_defines.h"
#include "platform/openfile_dialog.h"

#include "ui/global.h"

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include <fmt/format.h>
#include <iostream>

struct OAMData
{
    OAMData()
    {
        for (sf::Texture& tex : textures_)
            tex.create(8, 8);
    }

    void update()
    {
        Emulator& emulator = *Emulator::instance();
        PPU& ppu = *emulator.get_ppu();

        ::Image<8, 8> tile_buf;

        for (int i = 0; i < 64; ++i)
        {
            Sprite& sprite = sprites_[i];
            sprite = reinterpret_cast<const Sprite*>(ppu.oam_data())[i];

            ppu.sprite_img(sprite, tile_buf, static_cast<byte_t>(i));

            textures_[i].update(tile_buf.data());
        }
    }

    using Sprite = ::OAMSprite;
    std::array<Sprite, 64> sprites_;
    std::array<sf::Texture, 64> textures_;
};

struct PATData
{
    PATData()
    {
        for (sf::Texture& tex : pal_textures_)
            tex.create(4, 1);

        for (sf::Texture& tex : pat_textures_)
            tex.create(128, 128);
    }

    void update()
    {
        Emulator& emulator = *Emulator::instance();
        PPU& ppu = *emulator.get_ppu();

        for (int i = 0; i < 8; ++i)
        {
            Palette p = ppu.get_palette(static_cast<byte_t>(i));
            sf::Texture& tex = pal_textures_[i];

            for (int j = 0; j < 4; ++j)
            {
                Color c = p.get(j);
                byte_t* pixel = reinterpret_cast<byte_t*>(&c);
                tex.update(pixel, 1, 1, j, 0);
            }
        }

        Image<128, 128> buffer;
        ppu.patterntable_img(buffer, 0, ppu.get_palette(static_cast<byte_t>(pal_idx_)));
        pat_textures_[0].update(buffer.data());

        ppu.patterntable_img(buffer, 1, ppu.get_palette(static_cast<byte_t>(pal_idx_)));
        pat_textures_[1].update(buffer.data());
    }

    std::array<sf::Texture, 8> pal_textures_;
    std::array<sf::Texture, 2> pat_textures_;
    int pal_idx_ = 0;
};

struct NAMData
{
    NAMData()
    {
        texture_.create(512, 480);
    }

    void update()
    {
        Emulator& emulator = *Emulator::instance();
        const PPU& ppu = *emulator.get_ppu();

        Image<256, 240> image;
        for (byte_t i = 0; i < 4; ++i)
        {
            ppu.nametable_img(image, i);

            int x = (i % 2 == 0) ? 0 : 256;
            int y = (i / 2 == 0) ? 0 : 240;
            texture_.update(image.data(), 256, 240, x, y);
        }
    }

    sf::Texture texture_;
};

struct ImGuiGlobals
{
    static ImGuiGlobals& instance() 
    {
        static ImGuiGlobals instance;
        return instance;
    }

    bool show_cpu = false;
    bool show_ppu = false;
    bool imgui_examples = false;

    OAMData oam_data;
    PATData pat_data;
    NAMData nam_data;
};

void open_rom_dialog(Emulator& emulator)
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

std::string ws2s(std::wstring_view ws)
{
    std::string s;
#if IS_WINDOWS
    int wlen = static_cast<int>(ws.length());
    int len = WideCharToMultiByte(CP_ACP, 0, ws.data(), wlen, nullptr, 0, nullptr, 0);
    s.resize(len);
    WideCharToMultiByte(CP_ACP, 0, ws.data(), wlen, s.data(), len, nullptr, 0);
#endif
    return s;
}

void ui::imgui_mainmenu()
{
    Emulator& emulator = *Emulator::instance();
    
    if (imgui::BeginMainMenuBar())
    {
        if (imgui::BeginMenu("Emulation"))
        {
            if (imgui::MenuItem("Open ROM...", "Ctrl+O"))
                open_rom_dialog(emulator);

            auto recent_file = ui::get_recent_file(0);     
            imgui::BeginDisabled(!recent_file.has_value());

            if (imgui::BeginMenu("Recent"))
            {
                if (recent_file.has_value() && imgui::MenuItem(ws2s(recent_file.value().basename).c_str()))
                    emulator.read_rom(recent_file.value().fullpath);

                for (int i = 1; i < 5; ++i)
                {
                    recent_file = ui::get_recent_file(i);

                    if (!recent_file.has_value())
                        break;

                    if (imgui::MenuItem(ws2s(recent_file.value().basename).c_str()))
                        emulator.read_rom(recent_file.value().fullpath);
                }

                imgui::EndMenu();
            }

            imgui::EndDisabled();

            imgui::Separator();

            if (imgui::MenuItem("Reset", "Shift+R"))
                emulator.reset();

            if (imgui::MenuItem("Pause", nullptr, emulator.is_paused()))
                emulator.toggle_pause();

            imgui::EndMenu();
        }

        if (imgui::BeginMenu("Windows"))
        {
            ImGuiGlobals& globals = ImGuiGlobals::instance();

            if (imgui::MenuItem("CPU Debugger", nullptr, globals.show_cpu))
                globals.show_cpu = !globals.show_cpu;

            if (imgui::MenuItem("PPU Debugger", nullptr, globals.show_ppu))
                globals.show_ppu = !globals.show_ppu;

            if (imgui::MenuItem("Show Imgui Demo", nullptr, globals.imgui_examples))
                globals.imgui_examples = !globals.imgui_examples;

            imgui::EndMenu(); 
        }

        imgui::EndMainMenuBar();
    }
}

void debug_cpu();
void debug_ppu();

void ui::imgui_debugwindows()
{
    ImGuiGlobals& globals = ImGuiGlobals::instance();

    if (globals.show_cpu)
        debug_cpu();

    if (globals.show_ppu)
        debug_ppu();

    if (globals.imgui_examples)
        imgui::ShowDemoWindow();
}

void debug_cpu()
{
    using namespace imgui;

    if (Begin("CPU", &ImGuiGlobals::instance().show_cpu))
    {
        Emulator& emulator = *Emulator::instance();
        BUS& bus = *emulator.get_bus();
        CPU& cpu = *emulator.get_cpu();
        CPU_State const& cpu_state = cpu.get_state();

        if (BeginTable("layout_columns", 2))
        {
            TableNextRow();
            TableNextColumn();

            {
                StringBuilder sb;

                SeparatorText("Instructions");

                for (int offset = -5; offset <= 5; ++offset)
                {
                    sb.buf.push_back('\n');
                    emulator.disassembler_.render(sb, cpu_state.program_counter_, offset);
                }

                imgui::TextUnformatted(sb.buf.begin(), sb.buf.end());

            }

            TableNextColumn();
            {
                SeparatorText("CPU State");

                TextFmt("Flags: {}{}xx{}{}{}{}\n"
                    , (cpu_state.status_ & CPU_State::kNegative) ? 'N' : '-'
                    , (cpu_state.status_ & CPU_State::kOverflow) ? 'O' : '-'
                    , (cpu_state.status_ & CPU_State::kDecimal) ? 'D' : '-'
                    , (cpu_state.status_ & CPU_State::kIntDisable) ? 'I' : '-'
                    , (cpu_state.status_ & CPU_State::kZero) ? 'Z' : '-'
                    , (cpu_state.status_ & CPU_State::kCarry) ? 'C' : '-');

                TextFmt("A:{:02x} X:{:02x} Y:{:02x} SP:{:02x}\n"
                    , cpu_state.accumulator_
                    , cpu_state.register_x_
                    , cpu_state.register_y_
                    , cpu_state.stack_pointer_);

                NewLine();
                SeparatorText("Stepping");

                BeginDisabled(!emulator.is_ready());
                Debugger& debugger = emulator.debugger_;

                if (Button("Single"))
                    debugger.request_break(Debugger::MODE_CPU_FETCH);

                SameLine();
                if (Button("Line"))
                    debugger.request_break(Debugger::MODE_PPU_LINE);
                
                SameLine();
                if (Button("Frame"))
                    debugger.request_break(Debugger::MODE_PPU_FRAME);

                BeginDisabled(!emulator.is_debugging());

                if (Button("Continue"))
                    debugger.resume();

                EndDisabled();

                EndDisabled();
            }

            EndTable();
        }

        if (CollapsingHeader("Timings & VRAM", ImGuiTreeNodeFlags_DefaultOpen))
        {
            PPU const &ppu = *emulator.get_ppu();
            PPU_State const &ppu_state = ppu.get_state();
            address_t vram_addr = ppu.get_vram_addr();

            TextFmt("VBL timing: {}\n", emulator.cpu_cycle_last_vblank);
            TextFmt("CPU cycles: {}   PPU cycles: {}\n", emulator.cpu_cycle_per_frame, emulator.ppu_cycle_per_frame);
            TextFmt("PPU: scanline {} cycle {} vblank {}\n", ppu_state.scanline_, ppu_state.cycle_, ppu_state.is_in_vblank_);

            struct VRAM
            {
                address_t X : 5;
                address_t Y : 5;
                address_t N : 2;
                address_t y : 3;
            } v;

            v = *reinterpret_cast<VRAM *>(&vram_addr);
            byte_t N = v.N;
            byte_t X = v.X;
            byte_t Y = v.Y;
            byte_t x = 0;
            byte_t y = v.y;

            TextFmt("VRAM N:{} X:{} Y:{} x:{} y:{}\n", N, X, Y, x, y);
            NewLine();
        }

        if (CollapsingHeader("Test outputs"))
        {
            // Some tests write status value to $6000
            TextFmt("$6000: {:02x}", bus.read_cpu(0x6000));
            TextFmt("$6001: {:02x}", bus.read_cpu(0x6001));
            TextFmt("$6002: {:02x}", bus.read_cpu(0x6002));
            TextFmt("$6003: {:02x}", bus.read_cpu(0x6003));

            NewLine();

            std::string buf;
            {
                address_t it = 0x6004;
                byte_t c;
                for (;;)
                {
                    c = bus.read_cpu(it);
                    if (c == 0)
                        break;

                    ++it;
                    buf.push_back((char)c);
                }
            }

            TextFmt("$6004: {}", buf);
        }

        End();
    }
}

void debug_oam();
void debug_pat();
void debug_nam();

void debug_ppu()
{
    using namespace imgui;

    if (Begin("PPU", &ImGuiGlobals::instance().show_ppu))
    {
        if (BeginTabBar("-"))
        {
            if (BeginTabItem("OAM"))
            {
                debug_oam();
                EndTabItem();
            }

            if (BeginTabItem("PAT"))
            {
                debug_pat();
                EndTabItem();
            }

            if (BeginTabItem("NAM"))
            {
                debug_nam();
                EndTabItem();
            }

            EndTabBar();
        }
        End();
    }
}

void debug_oam()
{
    using namespace imgui;

    if (BeginChild("-"))
    {
        OAMData& oam_data = ImGuiGlobals::instance().oam_data;
        oam_data.update();

        auto entry = [&](int index)
        {
            const sf::Texture& tex = oam_data.textures_[index];
            const OAMData::Sprite& sprite = oam_data.sprites_[index];

            TextFmt("{:2}.", index + 1);

            SameLine();
            imgui::Image(tex, sf::Vector2f(16.f, 16.f));

            SameLine();
            TextFmt("{:02x} ({:3}, {:3}), {:02x}", sprite.tile_, sprite.x_, sprite.y_, sprite.att_);
        };

        if (BeginTable("-", 2))
        {
            for (int i = 0; i < 32; ++i)
            {
                TableNextRow();
                TableNextColumn();
                
                entry(i);

                TableNextColumn();

                entry(i + 32);
            }

            EndTable();
        }

        EndChild();
    }
}

void debug_pat()
{
    using namespace imgui;
    if (BeginChild("-"))
    {
        PATData& pat_data = ImGuiGlobals::instance().pat_data;
        pat_data.update();

        SeparatorText("Palettes");
        {
            std::string id;
            int selected = pat_data.pal_idx_;

            for (int i = 0; i < 8; ++i)
            {
                if (i == 0)
                    TextUnformatted("BG palettes: "); 

                if (i == 4)
                    TextUnformatted("FG palettes: ");

                SameLine();

                PushID(i);
                id = fmt::format("##Palette {}", i);

                ImVec2 cursor_pos = GetCursorPos();

                if (Selectable(id.c_str(), i == selected, 0, ImVec2{12 * 4, 12}))
                    selected = i;

                SetItemAllowOverlap();

                SetCursorPos({cursor_pos.x, cursor_pos.y});
                sf::Texture& tex = pat_data.pal_textures_[i];
                imgui::Image(tex, sf::Vector2f(12.f * 4.f, 12.f));

                PopID();
            }

            pat_data.pal_idx_ = selected;
        }

        SeparatorText("Pattern table 1");
        {
            sf::Texture& tex = pat_data.pat_textures_[0];
            imgui::Image(tex, sf::Vector2f(256.f, 256.f));
        }

        SeparatorText("Pattern table 2");
        {
            sf::Texture& tex = pat_data.pat_textures_[1];
            imgui::Image(tex, sf::Vector2f(256.f, 256.f));
        }

        EndChild();
    }
}

void debug_nam()
{
    using namespace imgui;

    if (BeginChild("-"))
    {
        ImGuiGlobals& globals = ImGuiGlobals::instance();
        NAMData& nam_data = globals.nam_data;

        nam_data.update();

        imgui::Image(nam_data.texture_, sf::Vector2f(512.f, 480.f));

        EndChild();
    }
}
