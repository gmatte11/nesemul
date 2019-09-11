#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "ram.h"

void BUS::write(address_t addr, byte_t value)
{
    if (ppu_.on_write(addr, value))
        return;

    if (ctrl_.on_write(addr, value))
        return;
    
    ram_.on_write(addr, value);
}

byte_t BUS::read(address_t addr)
{
    byte_t value;

    if (ppu_.on_read(addr, value))
        return value;

    if (ctrl_.on_read(addr, value))
        return value;

    ram_.on_read(addr, value);
    return value;
}
