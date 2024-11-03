#pragma once

#include "mappers/mapper.h"

// TxROM / MMC3
class M004 : public Mapper
{
public:
    M004(Cartridge& cart);

    address_t map_to_cpu_addr(address_t addr) const override;

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

    BankView get_bank(address_t addr) const;

private:
    Cartridge& cart_;

    struct Register : register_t<byte_t>
    {
        byte_t bank_select_ : 3;
        byte_t _ : 2;
        byte_t ram_enable_ : 1;
        byte_t prg_mode_ : 1;
        byte_t chr_mode_ : 1;
    } register_;

    byte_t irq_latch_ = 0;

    void bank_select_(byte_t value);
    void bank_data_(byte_t value);
};