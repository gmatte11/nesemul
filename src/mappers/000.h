#pragma once

#include "mapper.h"

// NROM
class M000 : public Mapper
{
public:
    M000(Cartridge* cart) : Mapper(cart) {}

    address_t map_to_cpu_addr(address_t addr) const override;

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

    std::pair<byte_t*, address_t> get_bank(address_t addr) const;
};