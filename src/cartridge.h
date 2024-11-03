#pragma once

#include "battery.h"
#include "types.h"

#include <ranges>

class INESReader;
class Mapper;

struct BankView
{
    std::span<byte_t> data_;
    address_t addr_ = 0x3333; // Never assigned to cartridge

    bool is_valid() const { return addr_ != 0x3333; }

    bool contains(address_t addr) const { return is_valid() && addr >= addr_ && addr < (addr_ + data_.size()); }

    bool is_mirror_of(BankView bank) const { return data_.data() == bank.data_.data(); }

    void write(int idx, byte_t value) 
    {  
        NES_ASSERT(is_valid());
        NES_ASSERT(idx >= 0 && idx < data_.size());
        data_[idx] = value;
    }

    void read(int idx, byte_t& value) const
    {
        NES_ASSERT(is_valid());
        NES_ASSERT(idx >= 0 && idx < data_.size());
        value = data_[idx];
    }
};

struct MemoryMap
{
    using Bank = BankView;

    std::array<Bank, 8> map_;

    Bank& operator[](int idx) { return map_[idx]; }
    const Bank& operator[](int idx) const { return map_[idx]; }

    Bank get_mapping(address_t addr)
    {
        for (Bank& bank : map_)
        {
            if (bank.contains(addr))
                return bank;
        }

        return {};
    }

    const Bank get_mapping(address_t addr) const
    {
        for (const Bank& bank : map_)
        {
            if (bank.contains(addr))
                return bank;
        }

        return {};
    }
};

class Cartridge
{
public:
    Cartridge();
    ~Cartridge();

    address_t map_to_cpu_addr(address_t addr);

    bool on_cpu_read(address_t addr, byte_t& value);
    bool on_cpu_write(address_t addr, byte_t value);
    bool on_ppu_read(address_t addr, byte_t& value);
    bool on_ppu_write(address_t addr, byte_t value);

    void load_roms(INESReader& reader);

    BankView get_bank(address_t addr) const;
    const MemoryMap& get_mapped_prg() const;
    const MemoryMap& get_mapped_chr() const;

    size_t calc_prg_offset(int idx, size_t bank_sz = prg_bank_sz) const;
    size_t calc_chr_offset(int idx, size_t bank_sz = chr_bank_sz) const;

    std::span<byte_t> get_prg_bank(int idx, size_t bank_sz = prg_bank_sz);
    std::span<const byte_t> get_prg_bank(int idx, size_t bank_sz = prg_bank_sz) const;
    std::span<byte_t> get_chr_bank(int idx, size_t bank_sz = chr_bank_sz);
    std::span<const byte_t> get_chr_bank(int idx, size_t bank_sz = chr_bank_sz) const;

    auto get_prg_banks() const { return std::views::chunk(prg_rom_, prg_bank_sz); }
    auto get_chr_banks() const { return std::views::chunk(chr_rom_, chr_bank_sz); }

private:
    std::unique_ptr<Mapper> mapper_;

public:
    static constexpr size_t prg_bank_sz = 0x4000;
    static constexpr size_t chr_bank_sz = 0x2000;

    std::vector<byte_t> prg_rom_;
    std::array<byte_t, 0x2000> wram_ {};
    std::vector<byte_t> chr_rom_;

    std::unique_ptr<Battery> battery_;
};