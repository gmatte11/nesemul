#pragma once

#include <memory>
#include <string>

#include "bus.h"
#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"
#include "controller.h"
#include "disassembler.h"

class Emulator
{
public:
    enum class Mode
    {
        RUN,
        PAUSED,
        STEP_ONCE,
        STEP_FRAME,
        STEP_LINE
    };

public:
    Emulator();
    ~Emulator();

    void read_rom(const std::string& filename);

    void reset();
    void update();

    bool is_paused() { return mode_ != Mode::RUN; }
    bool is_ready() { return cart_ != nullptr; }

    void step_once() { mode_ = Mode::STEP_ONCE; }
    void step_frame() { mode_ = Mode::STEP_FRAME; }
    void step_line() { mode_ = Mode::STEP_LINE; }
    void toggle_pause() { mode_ = (is_paused()) ? Mode::RUN : Mode::PAUSED; }

    Disassembler disassembler_;
    BUS* get_bus() const { return bus_.get(); }
    CPU* get_cpu() const { return cpu_.get(); }
    PPU* get_ppu() const { return ppu_.get(); }
    APU* get_apu() const { return apu_.get(); }

    void set_mode(Mode mode) { mode_ = mode; }
    Mode get_mode() const { return mode_; }

    void press_button(Controller::Button b);
    void release_button(Controller::Button b);

    static Emulator* instance() { return instance_; }

    uint64_t cpu_cycle_at_vblank = 0;
    uint64_t cpu_cycle_last_vblank = 0;
    uint64_t cpu_cycle_start_of_frame = 0;
    uint64_t cpu_cycle_per_frame = 0;

private:
    inline static Emulator* instance_ = nullptr;

    std::unique_ptr<BUS> bus_;
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<APU> apu_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<RAM> ram_;
    std::unique_ptr<Cartridge> cart_;

    Mode mode_ = Mode::RUN;
    int cycle_ = 0;
    int dma_cycle_counter_ = 0;
};