#include "clock.h"

#include "emulator.h"

#include <SFML/System.hpp>

static sf::Clock global_clock_;

time_unit Clock::now_ms()
{
    return global_clock_.getElapsedTime().asMilliseconds();
}

time_unit Clock::now_us()
{
    return global_clock_.getElapsedTime().asMicroseconds();
}

Timer::Timer(time_unit timeout)
{
    set_timeout(timeout);
}

bool Timer::expired(bool restart)
{
    time_unit t = Clock::now_us();
    if (t >= start_time_ + timeout_)
    {
        if (restart)
            start_time_ = t;

        return true;
    }

    return false;
}

void Timer::reset()
{
    start_time_ = Clock::now_us();
}

void Timer::set_timeout(time_unit timeout)
{
    timeout_ = timeout;
    if (started())
        start_time_ = Clock::now_us();
}

time_unit Timer::elapsed_us() const
{
    return std::max(Clock::now_us() - start_time_, 0ll);
}

time_unit FPSCounter::get_ppu_fps()
{
    time_unit now = Clock::now_ms();
    if ((now - last_time_) < 1000ll)
        return fps_;

    if (const PPU* ppu = Emulator::instance()->get_ppu())
    {
        time_unit frame = ppu->frame();
        if (frame != last_ppu_frame_)
        {
            fps_ = frame - last_ppu_frame_;
            last_ppu_frame_ = frame;
            last_time_ = now;
        }
    }

    return fps_;
}
