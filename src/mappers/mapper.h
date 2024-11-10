#pragma once

#include "types.h"
#include "cartridge.h"

#include <array>
#include <vector>

class Mapper
{
public:
    static Mapper* create(byte_t ines_code, Cartridge& cart);

public:
    Mapper() = default;

    virtual bool on_cpu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_cpu_write(address_t addr, byte_t value) = 0;

    virtual bool on_ppu_read(address_t addr, byte_t& value) = 0;
    virtual bool on_ppu_write(address_t addr, byte_t value) = 0;

    virtual void on_ppu_scanline(int scanline) {}

    virtual BankView get_cpu_mapped_bank(address_t addr) const
    {
        return prg_map_.get_mapping(map_to_cpu_addr(addr));
    }

    // TODO: still required?
    virtual address_t map_to_cpu_addr(address_t addr) const
    {
        return addr;
    }

    const MemoryMap& get_cpu_mapped_prg_banks() const
    {
        return prg_map_;
    }

    const MemoryMap& get_ppu_mapped_chr_banks() const
    {
        return chr_map_;
    }

protected:
    MemoryMap prg_map_;
    MemoryMap chr_map_;
};