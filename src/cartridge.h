#pragma once

#include "types.h"
#include "mapper.h"

class INESReader;

class Cartridge
{
public:
    using PRG_BANK = std::array<byte_t, 0x4000>;
    using CHR_BANK = std::array<byte_t, 0x2000>;

    Cartridge(byte_t ines_mapper_code);

    address_t map_to_cpu_addr(address_t addr)
    {
        if (addr >= 0x4020)
            return mapper_->map_to_cpu_addr(addr);
        return 0;
    }

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

    void load_roms(INESReader& reader);

    std::pair<byte_t*, address_t> get_bank(address_t addr) const;

    std::pair<const byte_t*, const byte_t*> get_prg_banks() const
    {
        return mapper_->get_cpu_mapped_prg_banks();
    }

    std::pair<const byte_t*, const byte_t*> get_chr_bank(int idx) const
    {
        if (idx == 0)
            return mapper_->get_ppu_mapped_chr_banks();

        if (idx == 1)
            return { wram_.data(), wram_.data() + 0x1000 };

        const byte_t* bank = chr_[idx - 2].data();
        return { bank, bank + 0x1000 };
    }

private:
    Mapper* mapper_;

public:
    std::vector<PRG_BANK> prg_rom_;
    std::array<byte_t, 0x2000> wram_ {};
    std::vector<CHR_BANK> chr_;
};