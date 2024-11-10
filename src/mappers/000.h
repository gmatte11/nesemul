#pragma once

#include "mappers/mapper.h"

// NROM
class M000 : public Mapper
{
public:
    M000(Cartridge& cart);

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

    address_t map_to_cpu_addr(address_t addr) const override;

private:
    BankView prg_l_;
    BankView prg_h_;
    BankView chr_;
};