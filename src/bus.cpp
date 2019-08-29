#include <bus.h>

void BUS::register_device(Device* device)
{
    for (Device*& dev : devices_)
        if (dev == nullptr)
        {
            dev = device;
            break;
        }
}

void BUS::write(address_t addr, byte_t value)
{
    for (Device* dev : devices_)
        if (dev->on_write(addr, value))
            break;
}

byte_t BUS::read(address_t addr)
{
    byte_t value;

    for (Device* dev : devices_)
        if (dev->on_read(addr, value))
            break;

    return value;
}

