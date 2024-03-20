#pragma once

class Emulator;
struct CPU_State;

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

private:
    Emulator& emulator_;
    Mode mode_ = MODE_RUNNING;
    Mode requested_mode_ = MODE_RUNNING;

    static Debugger* instance_;
};