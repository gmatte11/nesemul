#include "apu.h"
#include "emulator.h"

constexpr byte_t length_table[] = { 11, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
                                    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };

constexpr uint16_t dmc_rate_table[] = { 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54 };

enum : uint32_t
{
    SEQ_STEP_1 = 3728,
    SEQ_STEP_2 = 7456,
    SEQ_STEP_3 = 11185,
    SEQ_STEP_4 = 14914,
    SEQ_STEP_5 = 18640,
};

void APU::reset()
{
    cycle_ = 0;
    odd_cycle_ = false;
    five_steps_sequence_ = false;
    irq_inhibit_ = false;
    frame_irq_ = false;
    sequencer_change_delay_ = 0;

    pulse1_.on_ctrl(false);
    pulse2_.on_ctrl(false);
    triangle_.on_ctrl(false);
    noise_.on_ctrl(false);
    dmc_.on_ctrl(false);
}

void APU::step()
{
    bool quarter = false;
    bool half = false;

    if (!five_steps_sequence_ && cycle_ == SEQ_STEP_4)
        frame_irq_ = !irq_inhibit_;

    if (odd_cycle_)
    {
        if (!five_steps_sequence_ && cycle_ >= SEQ_STEP_4 + 1)
            cycle_ = 0;
        else if (five_steps_sequence_ && cycle_ >= SEQ_STEP_5 + 1)
            cycle_ = 0;

        switch (cycle_)
        {
        case SEQ_STEP_2: 
        case SEQ_STEP_5: 
            half = true;
            break;

        case SEQ_STEP_1: 
        case SEQ_STEP_3: 
            quarter = true;
            break;

        case SEQ_STEP_4:
            if (!five_steps_sequence_)
            {
                half = true;
                if (frame_irq_)
                    Emulator::instance()->get_cpu()->interrupt(false);
            }
            break;
        }
        
        ++cycle_;
    }

    if (sequencer_change_delay_ > 0 && --sequencer_change_delay_ == 0)
    {
        half |= five_steps_sequence_;
        cycle_ = 0;
    }

    if (quarter || half)
    {
        pulse1_.on_clock(half);
        pulse2_.on_clock(half);
        triangle_.on_clock(half);
        noise_.on_clock(half);
        dmc_.on_clock(half);
    }

    odd_cycle_ = !odd_cycle_;
}

bool APU::on_write(address_t addr, byte_t value)
{
    if (addr >= 0x4000 && addr <= 0x4003)
    {
        pulse1_.on_write(addr, value);
        return true;
    }

    if  (addr >= 0x4004 && addr <= 0x4007)
    {
        pulse2_.on_write(addr, value);
        return true;
    }

    if (addr >= 0x4008 && addr <= 0x400B)
    {
        triangle_.on_write(addr, value);
        return true;
    }

    if (addr >= 0x400C && addr <= 0x400F)
    {
        noise_.on_write(addr, value);
        return true;
    }

    if (addr >= 0x4010 && addr <= 0x4013)
    {
        dmc_.on_write(addr, value);
        return true;
    }
    
    if (addr == 0x4015)
    {
        Control ctrl;
        ctrl.set(value);

        pulse1_.on_ctrl(ctrl.pulse1);
        pulse2_.on_ctrl(ctrl.pulse2);
        triangle_.on_ctrl(ctrl.triangle);
        noise_.on_ctrl(ctrl.noise);
        dmc_.on_ctrl(ctrl.dmc);

        return true;
    }

    if (addr == 0x4017)
    {
        // Write is delayed by 3 or 4 CPU cycle depending on odd_cycle_.
        five_steps_sequence_ = value & 0xB0;
        irq_inhibit_ = value & 0x40;

        if (irq_inhibit_)
            frame_irq_ = false;

        sequencer_change_delay_ = 3 + (odd_cycle_) ? 1 : 0;

         return true;
    }

    return false;
}

bool APU::on_read(address_t addr, byte_t& value)
{
    if (addr == 0x4015)
    {
        Control status;
        status.pulse1 = (pulse1_.len_counter_ > 0);
        status.pulse2 = (pulse2_.len_counter_ > 0);
        status.triangle = (triangle_.len_counter_ > 0);
        status.noise = (noise_.len_counter_ > 0);
        status.dmc = dmc_.enabled_;

        status.frame_int = frame_irq_;
        frame_irq_ = false;

        value = status.get();

        return true;
    }
    return false;
}

void PulseChannel::on_clock(bool half_frame)
{
    if (half_frame)
    {
        if (len_counter_ > 0 && !len_halted_)
            --len_counter_;
    }
}

void PulseChannel::on_write(address_t addr, byte_t value)
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

void PulseChannel::on_ctrl(bool enable)
{
    len_enabled_ = enable;
    if (!enable)
        len_counter_ = 0;
}

void TriangleChannel::on_clock(bool half_frame)
{
    if (half_frame)
    {
        if (len_counter_ > 0 && !len_halted_)
            --len_counter_;
    }

    if (timer_ > 0)
        --timer_;
}

void TriangleChannel::on_write(address_t addr, byte_t value)
{
    switch (addr - 0x4008)
    {
    case 0:
        len_halted_ = value & 0x80;
        break;
    case 1:
        timer_ = (timer_ & 0xFF00) + value;
        break;
    case 2:
        timer_ = (timer_ & 0x00FF) + (static_cast<uint16_t>(value & 0x07) << 8);
        break;
    case 3:
        if (len_enabled_)
            len_counter_ = length_table[value >> 3];
        break;
    }
}

void TriangleChannel::on_ctrl(bool enable)
{
    len_enabled_ = enable;
    if (!enable)
        len_counter_ = 0;
}

void NoiseChannel::on_clock(bool half_frame)
{
    if (half_frame)
    {
        if (len_counter_ > 0 && !len_halted_)
            --len_counter_;
    }
}

void NoiseChannel::on_write(address_t addr, byte_t value)
{
    switch (addr - 0x400C)
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

void NoiseChannel::on_ctrl(bool enable)
{
    len_enabled_ = enable;
    if (!enable)
        len_counter_ = 0;
}

void DMChannel::on_clock(bool half_frame)
{
}

void DMChannel::on_write(address_t addr, byte_t value)
{
    switch (addr - 0x4010)
    {
    case 0:
        irq_enabled_ = value & 0x80;
        loop_ = value & 0x40;
        rate_idx_ = value & 0x0F;
        break;
    case 1:
        output_level_ = value & 0x7F;
        break;
    case 2:
        sample_addr_ = 0xC000 + static_cast<address_t>(value) * 64;
        break;
    case 3:
        sample_len_ = (value << 4) + 1;
        break;
    }
}

void DMChannel::on_ctrl(bool enable)
{
    enabled_ = enable;
}