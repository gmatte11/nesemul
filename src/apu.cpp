#include "apu.h"

constexpr byte_t length_table[] = { 10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
                                    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };

void APU::reset()
{
    cycle_ = 0;
    odd_cycle_ = false;
}

void APU::step()
{
    if (odd_cycle_)
    {
        if (sequencer_mode_ == 0 && cycle_ == 14915)
        {
            frame_irq_ = 1;
            cycle_ = 0;
        }
        else if (sequencer_mode_ == 1 && cycle_ == 18641)
        {
            cycle_ = 0;
        }

        bool quarter = false;
        bool half = false;

        switch (cycle_)
        {
        case 7456: // 2
        case 18640: // 5
            half = true;
            // fall-throught 
        case 3728: // 1
        case 11185: // 3
            quarter = true;
            break;

        case 14914: // 4
            quarter = half = (sequencer_mode_ == 0);
            break;
        }

        if (quarter || half)
        {
            pulse1.on_clock(half);
            pulse2.on_clock(half);
        }

        ++cycle_;
    }

    odd_cycle_ = !odd_cycle_;
}

bool APU::on_write(address_t addr, byte_t value)
{
    if (addr >= 0x4000 && addr <= 0x4003)
    {
        pulse1.on_write(addr, value);
        return true;
    }

    if  (addr >= 0x4004 && addr <= 0x4007)
    {
        pulse2.on_write(addr, value);
        return true;
    }

    if (addr >= 0x4008 && addr <= 0x400B)
    {
        ASSERT(false);
        return true;
    }

    if (addr >= 0x400C && addr <= 0x400F)
    {
        ASSERT(false);
        return true;
    }

    if (addr >= 0x4010 && addr <= 0x4013)
    {
        ASSERT(false);
        return true;
    }
    
    if (addr == 0x4015)
    {
        Control ctrl;
        ctrl.set(value);

        pulse1.on_ctrl(ctrl.pulse1);
        pulse2.on_ctrl(ctrl.pulse2);

        return true;
    }

    if (addr == 0x4017)
    {
        // TODO: write should be delayed by 3 or 4 CPU cycle depending on odd_cycle_.
        sequencer_mode_ = value & 0x80 ? 1 : 0;
        if (value & 0x40)
            frame_irq_ = 0;
        return true;
    }

    return false;
}

bool APU::on_read(address_t addr, byte_t& value)
{
    if (addr == 0x4015)
    {
        Control status;
        status.pulse1 = (pulse1.len_counter_ > 0);
        status.pulse2 = (pulse2.len_counter_ > 0);

        status.frame_int = frame_irq_ != 0;

        value = status.get();

        return true;
    }
    return false;
}

void APU::PulseChannel::on_clock(bool half_frame)
{
    if (half_frame)
    {
        if (len_counter_ > 0 && !len_halted_)
            --len_counter_;
    }
}

void APU::PulseChannel::on_write(address_t addr, byte_t value)
{
    switch (addr & 0x3)
    {
    case 0:
        len_halted_ = value & 0x20;
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        if (len_enabled_)
            len_counter_ = length_table[value >> 3];
        break;
    }
}

void APU::PulseChannel::on_ctrl(bool enabled)
{
    len_enabled_ = enabled;
    if (!enabled)
        len_counter_ = 0;
}