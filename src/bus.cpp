#include "bus.h"
#include "cpu.h"
#include "apu.h"
#include "ppu.h"
#include "ram.h"
#include "cartridge.h"

void BUS::write_cpu(address_t addr, byte_t value)
{
    if (ppu_.on_write_cpu(addr, value)) ;
    else if (apu_.on_write(addr, value)) ;
    else if (ctrl_.on_write(addr, value)) ;
    else if (cart_ && cart_->on_cpu_write(addr, value)) ;
    else ram_.on_write(addr, value);
}

byte_t BUS::read_cpu(address_t addr) const
{
    byte_t value = 0;

    if (ppu_.on_read_cpu(addr, value)) ;
    else if (apu_.on_read(addr, value)) ;
    else if (ctrl_.on_read(addr, value)) ;
    else if (cart_ && cart_->on_cpu_read(addr, value)) ;
    else ram_.on_read(addr, value);
    
    return value;
}

void BUS::write_ppu(address_t addr, byte_t value)
{
    if (cart_ && cart_->on_ppu_write(addr, value)) ;
    else ppu_.on_write_ppu(addr, value);
}

byte_t BUS::read_ppu(address_t addr) const
{
    byte_t value = 0;

    if (cart_ && cart_->on_ppu_read(addr, value)) ;
    else ppu_.on_read_ppu(addr, value);

    return value;
}
