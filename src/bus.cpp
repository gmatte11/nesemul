#include "bus.h"
#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"

void BUS::write(address_t addr, byte_t value)
{
    if (ppu_.on_write(addr, value)) ;
    else if (apu_.on_write(addr, value)) ;
    else if (ctrl_.on_write(addr, value)) ;
    else if (ram_.on_write(addr, value)) ;
    else cart_.on_cpu_write(addr, value);
}

byte_t BUS::read(address_t addr)
{
    byte_t value = 0;

    if (ppu_.on_read(addr, value)) ;
    else if (apu_.on_read(addr, value)) ;
    else if (ctrl_.on_read(addr, value)) ;
    else if (ram_.on_read(addr, value)) ;
    else cart_.on_cpu_read(addr, value);
    
    return value;
}
