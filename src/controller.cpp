#include "controller.h"

bool Controller::on_read(address_t addr, byte_t& value)
{
    if (addr == 0x4016)
    {
        if (read_state_ == 0xFF)
        {
            value = state() & 0x01;
        }
        else if (read_state_ < 8)
        {
            value = (state() >> read_state_) & 0x01;
            ++read_state_;
        }
        else
        {
            value = 0x01;
        }
        return true;
    }

    // TODO: Implement controller 2
    if (addr == 0x4017)
    {
        value = 0x00;
        return true;
    }

    return false;
}

bool Controller::on_write(address_t addr, byte_t value)
{
    if (addr == 0x4016)
    {
        read_state_ = (value & 0x01) ? 0xFF : 0;
        return  true;
    }

    return false;
}