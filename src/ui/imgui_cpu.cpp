#include "imgui_cpu.h"

#include "emulator.h"
#include "bus.h"
#include "cpu.h"
#include "ppu.h"

#include "ui/global.h"
#include "ui/imgui.h"
#include "ui/imgui_debugger.h"
#include "ui/imgui_mem.h"

static ui::CPUData& globals() { return *ui::Globals::get().cpu_data_; }

void cpu_tab();
void ram_tab();

void ui::imgui_cpu_window()
{
    using namespace imgui;

    if (!globals().show_window_)
        return;

    if (Begin("CPU", &globals().show_window_))
    {
        if (BeginTabBar("-"))
        {
            if (BeginTabItem("CPU"))
            {
                cpu_tab();
                EndTabItem();
            }

            if (BeginTabItem("RAM"))
            {
                ram_tab();
                EndTabItem();
            }

            EndTabBar();
        }

        End();
    }
}

void cpu_tab()
{
    using namespace imgui;

    if (BeginChild("-"))
    {
        Emulator& emulator = *Emulator::instance();
        const BUS& bus = *emulator.get_bus();

        ui::imgui_debugger();

        if (CollapsingHeader("Timings & VRAM", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const PPU& ppu = *emulator.get_ppu();
            const PPU_State& ppu_state = ppu.get_state();
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

        EndChild();
    }
}

void ram_tab()
{
    if (ImGui::BeginChild("-"))
    {
        ui::imgui_mem_view();
        ImGui::EndChild();
    }
}