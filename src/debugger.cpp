#include "debugger.h"

#include "emulator.h"
#include "types.h"

#include <algorithm>

Debugger* Debugger::instance_ = nullptr;

bool Breakpoint::check(const CPU_State& state) const
{
    switch (reason_)
    {
    case Reason::Addr:
        return state.program_counter_ == std::get<address_t>(break_value_);
    case Reason::Opcode:
        return state.instr_.opcode == std::get<byte_t>(break_value_);
    case Reason::Flag:
    {
        const byte_t status = std::get<byte_t>(break_value_); 
        return state.status_ == status;
    }
    }

    return false;
}

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

    if (std::ranges::any_of(breakpoints_, [&](const Breakpoint& cb) { return cb.check(state); }))
        mode_ = MODE_CPU_FETCH;
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
