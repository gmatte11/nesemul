#pragma once

#include <types.h>

#include <array>

struct Device
{
    virtual bool on_write(address_t addr, byte_t operand) = 0;
    virtual bool on_read(address_t addr, byte_t& output) = 0;
};

class BUS
{
public:
    void register_device(Device* device);

    void write(address_t addr, byte_t value);
    byte_t read(address_t addr);

private:
    std::array<Device*, 2> devices_ = { nullptr };
};
