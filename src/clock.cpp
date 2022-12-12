#include "clock.h"

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