#pragma once

#include "types.h"

class Controller
{
public:
    enum Button : byte_t
    {
        A       = 0b00000001, 
        B       = 0b00000010, 
        Select  = 0b00000100, 
        Start   = 0b00001000,
        Up      = 0b00010000,
        Down    = 0b00100000,
        Left    = 0b01000000,
        Right   = 0b10000000
    };

    bool on_read(address_t addr, byte_t& value) const;
    bool on_write(address_t addr, byte_t value);

    bool is_pressed(Button button) const { return (state() & button) != 0; }
    void press(Button button) { state() |= button; }
    void release(Button button) { state() &= ~button; }

private:
    byte_t buttons_state_ = 0;
    mutable byte_t read_state_ = 0;

    byte_t& state() { return buttons_state_; }
    byte_t state() const { return buttons_state_; }
};