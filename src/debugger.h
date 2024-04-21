#pragma once

#include "types.h"
#include "utils.h"

#include <variant>
#include <vector>
#include <ranges>

class Emulator;
struct CPU_State;

struct Breakpoint
{
    NAMED_TYPED_ENUM(Reason, uint8_t, Addr, Opcode, Flag);
    std::variant<byte_t, address_t> break_value_{ 0_addr };
    Reason reason_;

    bool check(const CPU_State& state) const;
};

static_assert(sizeof(Breakpoint) <= sizeof(int64_t));

class Debugger
{
public:
    enum Mode
    {
        MODE_RUNNING,
        MODE_CPU_FETCH,
        MODE_PPU_LINE,
        MODE_PPU_FRAME,
    };

public:
    Debugger(Emulator& emulator);

    static Debugger* instance() { return instance_; }

    Mode get_mode() const { return mode_; }

    void request_break(Mode mode);
    void break_now();
    void resume();

    void on_cpu_fetch(const CPU_State& state);

    void on_ppu_frame();
    void on_ppu_line();

    std::vector<Breakpoint> breakpoints_;

private:
    Emulator& emulator_;
    Mode mode_ = MODE_RUNNING;
    Mode requested_mode_ = MODE_RUNNING;

    static Debugger* instance_;
};