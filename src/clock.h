#pragma once

#include <chrono>

using time_unit = int64_t;
inline constexpr time_unit sixtieth_us = 1000000ll/60;

class Clock
{
public:
    static time_unit now_ms();
    static time_unit now_us();
};

class Timer
{
public:
    Timer(time_unit timeout = 0);
    bool expired(bool restart = true);
    void reset();

    bool started() const { return timeout_ > 0; }

    void set_timeout(time_unit timeout);
    time_unit get_timeout() const { return timeout_; }

    time_unit elapsed_us() const;

private:
    time_unit timeout_ = 0;
    time_unit start_time_ = 0;
};

class FPSCounter
{
public:
    time_unit get_ppu_fps();

private:
    time_unit last_ppu_frame_ = 0;
    time_unit fps_ = 0;
    time_unit last_time_ = 0;
};