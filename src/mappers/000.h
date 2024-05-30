#pragma once

#include "mapper.h"

// NROM
class M000 : public Mapper
{
public:
    M000();

    void post_load(Cartridge& cart) override;

    address_t map_to_cpu_addr(address_t addr) const override;

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

    std::pair<byte_t*, address_t> get_bank(address_t addr) const;

    byte_t*& prg_l_;
    byte_t*& prg_h_;
    byte_t*& chr_l_;
    byte_t*& chr_h_;
};