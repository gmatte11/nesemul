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
        byte_t* mem_ = nullptr;

        address_t addr_ = 0x3333; // Never mapped to cartridge
        address_t size_ = 0;

        int rom_bank_idx_ = - 1;
        address_t bank_offset_ = 0;
    };

    std::array<Bank, 8> map_;

    Bank& operator[](int idx) { return map_[idx]; }
    const Bank& operator[](int idx) const { return map_[idx]; }

    std::optional<std::reference_wrapper<Bank>> get_mapping(address_t addr)
    {
        for (Bank& mapping : map_)
        {
            if (addr >= mapping.addr_ && addr < mapping.addr_ + mapping.size_)
                return mapping;
        }

        return {};
    }

    std::optional<std::reference_wrapper<const Bank>> get_mapping(address_t addr) const
    {
        for (const Bank& mapping : map_)
        {
            if (addr >= mapping.addr_ && addr < mapping.addr_ + mapping.size_)
                return mapping;
        }

        return {};
    }

    byte_t* map_to_mem(address_t addr)
    {
        auto opt_bank = get_mapping(addr);

        if (opt_bank)
        {
            Bank& bank = opt_bank.value();
            if (bank.mem_ != nullptr)
                return bank.mem_ + (addr - bank.addr_);
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