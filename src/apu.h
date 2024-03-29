#pragma once

#include "types.h"

class BUS;

struct Control : public register_t<byte_t>
{
    bool pulse1 : 1; // (bit 0)
    bool pulse2 : 1; // (bit 1)
    bool triangle : 1; // (bit 2)
    bool noise : 1; // (bit 3)
    bool dmc : 1; // (bit 4)
    bool dummy : 1; // (bit 5)
    bool frame_int : 1; // (bit 6)
    bool dmc_int : 1; // (bit 7)
};

struct PulseChannel
{
    struct Timer : public register_t<address_t>
    {
        address_t time : 11; // (bits 0-10)
        byte_t enabled : 1; // (bit 11)
    } timer;

    bool len_enabled_ : 1 {};
    bool len_halted_ : 1 {};
    byte_t len_counter_ {};

    void on_clock(bool half_frame);
    void on_write(address_t addr, byte_t value);
    void on_ctrl(bool enable);
};

struct TriangleChannel
{
    bool len_enabled_ : 1;
    bool len_halted_ : 1;
    byte_t len_counter_;
    uint16_t timer_ = 0;

    void on_clock(bool half_frame);
    void on_write(address_t addr, byte_t value);
    void on_ctrl(bool enable);
};

struct NoiseChannel
{
    bool len_enabled_ : 1;
    bool len_halted_ : 1;
    byte_t len_counter_;

    void on_clock(bool half_frame);
    void on_write(address_t addr, byte_t value);
    void on_ctrl(bool enable);
};

struct DMChannel
{
    address_t sample_addr_;
    uint16_t sample_len_;
    uint8_t output_level_;

    uint8_t rate_idx_ = 0;
    bool enabled_ : 1 {};
    bool irq_enabled_ : 1 {};
    bool loop_ : 1 {};

    void on_clock(bool half_frame);
    void on_write(address_t addr, byte_t value);
    void on_ctrl(bool enable);
};

class APU
{
public:
    APU() = default;

    void step();
    void reset();

    bool on_write(address_t addr, byte_t value);
    bool on_read(address_t addr, byte_t& value);

private:
    PulseChannel pulse1_;
    PulseChannel pulse2_;
    TriangleChannel triangle_;
    NoiseChannel noise_;
    DMChannel dmc_;

    uint32_t cycle_ = 0;
    bool odd_cycle_ = false;
    bool five_steps_sequence_ = false;
    bool irq_inhibit_ = false;
    bool frame_irq_ = false;

    uint8_t sequencer_change_delay_;
};