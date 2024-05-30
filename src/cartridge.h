#pragma once

#include "battery.h"
#include "types.h"

#include <ranges>

class INESReader;
class Mapper;

struct MemoryMap
{
    struct Bank
    {
        address_t addr_ = 0x3333; // Never mapped to cartridge
        address_t size_ = 0;
        byte_t* mem_ = nullptr;
    };

    std::array<Bank, 4> map_;

    bool is_mapped(address_t addr) const
    {
        bool mapped = false;
        for (const Bank& mapping : map_)
        {
            mapped = mapped 
                || (addr >= mapping.addr_ 
                && addr < mapping.addr_ + mapping.size_ 
                && mapping.mem_ != nullptr);
        }
        return mapped;
    }

    byte_t* map(address_t addr)
    {
        for (const Bank& mapping : map_)
        {
            if (addr >= mapping.addr_ && addr < mapping.addr_ + mapping.size_)
                return mapping.mem_ + (addr - mapping.addr_);
        }

        return nullptr;
    }
};

class Cartridge
{
public:
    Cartridge(byte_t ines_mapper_code);

    address_t map_to_cpu_addr(address_t addr);

    bool on_cpu_read(address_t addr, byte_t& value);
    bool on_cpu_write(address_t addr, byte_t value);
    bool on_ppu_read(address_t addr, byte_t& value);
    bool on_ppu_write(address_t addr, byte_t value);

    void load_roms(INESReader& reader);

    std::pair<byte_t*, address_t> get_bank(address_t addr) const;
    const MemoryMap& get_mapped_prg() const;
    const MemoryMap& get_mapped_chr() const;

    byte_t* get_prg_bank(int idx) const;
    byte_t* get_chr_bank(int idx) const;

    auto get_prg_banks() const { return std::views::chunk(prg_rom_, prg_bank_sz); }
    auto get_chr_banks() const { return std::views::chunk(chr_, chr_bank_sz); }

private:
    Mapper* mapper_;

public:
    static constexpr size_t prg_bank_sz = 0x4000;
    static constexpr size_t chr_bank_sz = 0x2000;

    std::vector<byte_t> prg_rom_;
    std::array<byte_t, 0x2000> wram_ {};
    std::vector<byte_t> chr_;

    std::unique_ptr<Battery> battery_;
};