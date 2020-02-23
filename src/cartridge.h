#pragma once

#include "types.h"
#include "mapper.h"

#include <iosfwd>
using bifstream = std::basic_ifstream<byte_t>;

class Cartridge
{
public:
    using PRG_BANK = std::array<byte_t, 0x4000>;
    using CHR_BANK = std::array<byte_t, 0x2000>;

    Cartridge(byte_t ines_mapper_code);

    bool on_cpu_read(address_t addr, byte_t& value) 
    { 
        if (addr >= 0x4020)
            return mapper_->on_cpu_read(addr, value);
        return false;
    }

    bool on_cpu_write(address_t addr, byte_t value) 
    { 
        if (addr >= 0x4020)
            return mapper_->on_cpu_write(addr, value); 
        return false;
    }

    bool on_ppu_read(address_t addr, byte_t& value) { return mapper_->on_ppu_read(addr, value); }
    bool on_ppu_write(address_t addr, byte_t value) { return mapper_->on_ppu_write(addr, value); }

    void load_roms(bifstream& ifs, byte_t prg_rom_banks, byte_t chr_rom_banks, byte_t prg_ram_banks);

    std::pair<byte_t*, address_t> get_bank(address_t addr) const;

private:
    Mapper* mapper_;

public:
    std::vector<PRG_BANK> prg_rom_;
    std::array<byte_t, 0x2000> wram_;
    std::vector<CHR_BANK> chr_rom_;
    std::array<byte_t, 0x2000> vram_;
};