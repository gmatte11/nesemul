#include "imgui.h"

#include "emulator.h"
#include "utils.h"

#include "platform/platform_defines.h"
#include "platform/openfile_dialog.h"

#include "ui/global.h"
#include "ui/imgui_cpu.h"
#include "ui/imgui_ppu.h"

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include <fmt/format.h>
#include <iostream>

static bool imgui_examples = false;

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
            PPUData& ppu_globals = *Globals::get().ppu_data_;
            CPUData& cpu_globals = *Globals::get().cpu_data_;

            if (imgui::MenuItem("CPU Debugger", nullptr, cpu_globals.show_window_))
                cpu_globals.show_window_ = !cpu_globals.show_window_;

            if (imgui::MenuItem("PPU Debugger", nullptr, ppu_globals.show_window_))
                ppu_globals.show_window_ = !ppu_globals.show_window_;

            if (imgui::MenuItem("Show Imgui Demo", nullptr, imgui_examples))
                imgui_examples = !imgui_examples;

            imgui::EndMenu(); 
        }

        imgui::EndMainMenuBar();
    }
}

void ui::imgui_debugwindows()
{
    imgui_cpu_window();
    imgui_ppu_window();

    if (imgui_examples)
        imgui::ShowDemoWindow();
}