#pragma once

#include "mapper.h"

// SxROM
class M001 : public Mapper
{
public:
    M001();
    void post_load(Cartridge& cart) override;

    address_t map_to_cpu_addr(address_t addr) const override;

    bool on_cpu_read(address_t addr, byte_t& value) override;
    bool on_cpu_write(address_t addr, byte_t value) override;

    bool on_ppu_read(address_t addr, byte_t& value) override;
    bool on_ppu_write(address_t addr, byte_t value) override;

    std::pair<byte_t*, address_t> get_bank(address_t addr) const;
private:
    Cartridge* cart_;

    byte_t*& prg_l_;
    byte_t*& prg_h_;
    byte_t*& chr_l_;
    byte_t*& chr_h_;

    struct : public register_t<byte_t> 
    { 
        byte_t latch : 5;
        byte_t writes : 3;
    } register_ = {};
    byte_t control_ = 0;
    
    void chr_low_switch();
    void chr_high_switch();
    void prg_switch();
};