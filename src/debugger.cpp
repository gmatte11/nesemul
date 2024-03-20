#include "debugger.h"

#include "emulator.h"
#include "types.h"

Debugger* Debugger::instance_ = nullptr;

Debugger::Debugger(Emulator& emulator)
    : emulator_(emulator)
{
    NES_ASSERT(instance_ == nullptr);
    instance_ = this;
}

void Debugger::request_break(Mode mode)
{
    requested_mode_ = mode;
    mode_ = MODE_RUNNING;
}

void Debugger::break_now()
{
    mode_ = MODE_CPU_FETCH;
}

void Debugger::resume()
{
    mode_ = MODE_RUNNING;
    requested_mode_ = MODE_RUNNING;
}

void Debugger::on_cpu_fetch(const CPU_State& state)
{
    if (requested_mode_ == MODE_CPU_FETCH)
        mode_ = requested_mode_;
}

void Debugger::on_ppu_frame()
{
    if (requested_mode_ == MODE_PPU_FRAME)
        mode_ = requested_mode_;
}

void Debugger::on_ppu_line()
{
    if (requested_mode_ == MODE_PPU_LINE)
        mode_ = requested_mode_;
}
