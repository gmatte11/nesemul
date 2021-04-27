#pragma once

#include <memory>
#include <string>

#include "bus.h"
#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"
#include "disassembler.h"

class Emulator
{
public:
    enum class Mode
    {
        RUN,
        PAUSED,
        STEP_ONCE,
        STEP_FRAME
    };

public:
    Emulator();

    void read_rom(const std::string& filename);

    void reset();

    int run();

    bool is_paused() { return mode_ != Mode::RUN; }

    void step_once() { mode_ = Mode::STEP_ONCE; }
    void step_frame() { mode_ = Mode::STEP_FRAME; }
    void toggle_pause() { mode_ = (is_paused()) ? Mode::RUN : Mode::PAUSED; }

    Disassembler disassembler_;
    BUS* get_bus() const { return bus_.get(); }

    void set_mode(Mode mode) { mode_ = mode; }
    Mode get_mode() const { return mode_; }

private:
    std::unique_ptr<BUS> bus_;
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<APU> apu_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<RAM> ram_;
    std::unique_ptr<Cartridge> cart_;

    Mode mode_ = Mode::RUN;
    int cycle_ = 0;
    int dma_cycle_counter = 0;
};