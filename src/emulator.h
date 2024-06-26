#pragma once

#include <memory>
#include <string_view>

#include "bus.h"
#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"
#include "controller.h"
#include "debugger.h"
#include "disassembler.h"

class Emulator
{
public:
    Emulator();
    ~Emulator();

    void read_rom(std::wstring_view filename);

    void reset();
    void update();

    bool is_stepping() const { return !is_paused() && !is_debugging(); }
    bool is_debugging() const { return debugger_.get_mode() != Debugger::MODE_RUNNING; }
    bool is_paused() const { return paused_; }
    bool is_ready() const { return cart_ != nullptr; }

    void toggle_pause();

    Disassembler disassembler_;
    Debugger debugger_;

    BUS* get_bus() const { return bus_.get(); }
    CPU* get_cpu() const { return cpu_.get(); }
    PPU* get_ppu() const { return ppu_.get(); }
    APU* get_apu() const { return apu_.get(); }
    Cartridge* get_cart() const { return cart_.get(); }

    void press_button(Controller::Button b);
    void release_button(Controller::Button b);

    static Emulator* instance() { return instance_; }

    uint64_t cpu_cycle_at_vblank = 0;
    uint64_t cpu_cycle_last_vblank = 0;
    uint64_t cpu_cycle_start_of_frame = 0;
    uint64_t cpu_cycle_per_frame = 0;
    uint64_t ppu_cycle_start_of_frame = 0;
    uint64_t ppu_cycle_per_frame = 0;

private:
    void clock_ppu_();
    void clock_cpu_();

    inline static Emulator* instance_ = nullptr;

    std::unique_ptr<BUS> bus_;
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<APU> apu_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<RAM> ram_;
    std::unique_ptr<Cartridge> cart_;

    uint64_t cycle_ = 0;
    int dma_cycle_counter_ = 0;

    bool paused_ = false;
};