#pragma once

#include "types.h"

class BUS;

class APU
{
public:
    APU() = default;

    void step();
    void reset();

    bool on_write(address_t addr, byte_t value);
    bool on_read(address_t addr, byte_t& value);

private:
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

        bool len_enabled_ : 1;
        bool len_halted_ : 1;
        byte_t len_counter_;

        void on_clock(bool half_frame);
        void on_write(address_t addr, byte_t value);
        void on_ctrl(bool enabled);
    };

    PulseChannel pulse1;
    PulseChannel pulse2;

    byte_t sequencer_mode_;
    byte_t frame_irq_;

    uint32_t cycle_ = 0;
    bool odd_cycle_ = false;
};