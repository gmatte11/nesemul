#pragma once

#include "mappers/mapper.h"

// TxROM / MMC3
class M004 : public Mapper
{
public:
    M004(Cartridge& cart);

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

    void on_ppu_scanline(int scanline) override;

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
    byte_t irq_counter_ = 0;
    bool irq_enabled_ = false;
    bool irq_reload_ = false;

    void on_bank_select_(byte_t value);
    void on_bank_data_(byte_t value);
    void on_mirroring_(byte_t value);
    void on_prg_ram_protect_(byte_t value);
    void on_irq_latch_(byte_t value);
    void on_irq_reload_();
    void on_irq_disable_();
    void on_irq_enable_();
};