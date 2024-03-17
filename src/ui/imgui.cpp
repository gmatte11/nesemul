#include "imgui.h"

#include "emulator.h"
#include "utils.h"

#include "platform/openfile_dialog.h"

#include <fmt/format.h>
#include <iostream>

struct ImGuiGlobals
{
    static ImGuiGlobals& instance() 
    {
        static ImGuiGlobals instance;
        return instance;
    }

    bool show_cpu = false;
    bool show_oam = false;
    bool imgui_examples = false;
};

void open_rom_dialog(Emulator& emulator)
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

void ui::imgui_mainmenu()
{
    Emulator& emulator = *Emulator::instance();
    
    if (imgui::BeginMainMenuBar())
    {
        if (imgui::BeginMenu("Emulation"))
        {
            if (imgui::MenuItem("Open ROM...", "Ctrl+O"))
                open_rom_dialog(emulator);

            if (imgui::BeginMenu("Recent"))
            {
                imgui::MenuItem("Recent 1");
                imgui::MenuItem("Recent 2");
                imgui::MenuItem("Recent 3");
                imgui::EndMenu();
            }

            imgui::Separator();

            if (imgui::MenuItem("Reset"))
                emulator.reset();

            if (imgui::MenuItem("Pause", nullptr, emulator.is_paused()))
                emulator.toggle_pause();

            imgui::EndMenu();
        }

        if (imgui::BeginMenu("Debug"))
        {
            ImGuiGlobals& globals = ImGuiGlobals::instance();

            if (imgui::MenuItem("Debugger", nullptr, globals.show_cpu))
                globals.show_cpu = !globals.show_cpu;

            if (imgui::MenuItem("Debug OAM", nullptr, globals.show_oam))
                globals.show_oam = !globals.show_oam;

            if (imgui::MenuItem("Show Imgui Demo", nullptr, globals.imgui_examples))
                globals.imgui_examples = !globals.imgui_examples;

            imgui::EndMenu(); 
        }

        imgui::EndMainMenuBar();
    }
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

        BeginGroup();

        if (BeginTable("layout_columns", 2))
        {
            TableNextRow();
            TableNextColumn();

            if (BeginChild("instr_child"))
            {
                StringBuilder sb;

                SeparatorText("Instructions");

                for (int offset = -5; offset <= 5; ++offset)
                {
                    sb.buf.push_back('\n');
                    emulator.disassembler_.render(sb, cpu_state.program_counter_, offset);
                }

                imgui::TextUnformatted(sb.buf.begin(), sb.buf.end());

                NewLine();
                SeparatorText("Tests outputs");

                // Some tests write status value to $6000
                TextFmt("$6000: {:02x}", bus.read_cpu(0x6000));
                TextFmt("$6001: {:02x}", bus.read_cpu(0x6001));
                TextFmt("$6002: {:02x}", bus.read_cpu(0x6002));
                TextFmt("$6003: {:02x}", bus.read_cpu(0x6003));

                NewLine();

                sb.buf.clear();
                {
                    address_t it = 0x6004;
                    byte_t c;
                    for (;;)
                    {
                        c = bus.read_cpu(it);
                        if (c == 0)
                            break;

                        ++it;
                        sb.buf.push_back((char)c);
                    }
                }

                TextFmt("$6004: {}", fmt::to_string(sb.buf));

                EndChild();
            }

            TableNextColumn();
            if (BeginChild("opt_child"))
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

                EndChild();
            }

            EndTable();
        }
    }
}

void debug_oam()
{
    if (imgui::Begin("OAM", &ImGuiGlobals::instance().show_oam))
    {
        imgui::TextUnformatted("OAM");




        imgui::End();
    }
}

void ui::imgui_debugwindows()
{
    ImGuiGlobals& globals = ImGuiGlobals::instance();

    if (globals.show_cpu)
        debug_cpu();

    if (globals.show_oam)
        debug_oam();

    if (globals.imgui_examples)
        imgui::ShowDemoWindow();
}