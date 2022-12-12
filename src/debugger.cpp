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
    NES_ASSERT(mode != MODE_RUNNING);
    mode_ = mode;
    emulator_.debug_step();
}

void Debugger::resume()
{
    mode_ = MODE_RUNNING;
}

void Debugger::on_cpu_fetch()
{
    if (mode_ == MODE_CPU_FETCH)
        emulator_.debug_break();
}

void Debugger::on_ppu_frame()
{
    if (mode_ == MODE_PPU_FRAME)
        emulator_.debug_break();
}

void Debugger::on_ppu_line()
{
    if (mode_ == MODE_PPU_LINE)
        emulator_.debug_break();
}
