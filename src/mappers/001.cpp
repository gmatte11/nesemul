#include "001.h"
#include "cartridge.h"
#include "emulator.h"

M001::M001(Cartridge& cart)
    : cart_(cart)
{
    prg_l_ = { cart.get_prg_bank(0), 0x8000 };
    prg_map_[0] = prg_l_;

    const size_t offset_l = cart.calc_prg_offset(0);
    const size_t offset_h = cart.calc_prg_offset(-1);

    if (offset_h != offset_l)
    {
        prg_h_ = { cart.get_prg_bank(-1), 0xC000 };
        prg_map_[1] = prg_h_;
    }
    else
    {
        prg_h_ = prg_l_;
    }

    // A single bank of CHR RAM is created when no CHR ROM is available in the cartridge.
    if (cart.chr_rom_.empty())
        cart.chr_rom_.resize(Cartridge::chr_bank_sz);

    chr_l_ = { cart.get_chr_bank(0, 0x1000), 0x0000 };
    chr_map_[0] = chr_l_;

    chr_h_ = { cart.get_chr_bank(1, 0x1000), 0x1000 };
    chr_map_[1] = chr_h_;
}

address_t M001::map_to_cpu_addr(address_t addr) const
{
    return addr;
}

bool M001::on_cpu_read(address_t addr, byte_t& value) 
{
    if (addr >= 0x6000 && addr < 0x8000)
    {
        if (!(cart_.battery_ && cart_.battery_->read(addr, value)))
            value = cart_.wram_[addr & 0x1FFF];
        return true;
    }

    if (addr >= 0x8000 && addr < 0xC000)
    {
        prg_l_.read(addr & 0x3FFF, value);
        return true;
    }

    if (addr >= 0xC000)
    {
        prg_h_.read(addr & 0x3FFF, value);
        return true;
    }

    return false;
}

bool M001::on_cpu_write(address_t addr, byte_t value)
{
    if (addr >= 0x6000 && addr < 0x8000)
    {
        if (!(cart_.battery_ && cart_.battery_->write(addr, value)))
            cart_.wram_[addr & 0x1FFF] = value;
        return true;
    }

    if (addr >= 0x8000)
    {
        const bool reset = (0x80 & value);
        const int op = (addr & 0x6000) >> 13;

        if (reset)
        {
            register_ = {};
            if (op == 0)
                control_ |= 0xC;
            else if (op == 3)
                prg_h_.data_ = cart_.get_prg_bank(-1);

            return true;
        }

        (register_.latch >>= 1) |= ((0x1 & value) << 4);
        ++register_.writes;

        if (register_.writes >= 5)
        {
            switch (op)
            {
            case 0: // control
            {
                control_ = register_.latch;

                const byte_t mirroring = control_ & 0x3;
                NT_Mirroring m = NT_Mirroring::Single;
                if (mirroring == 2) m = NT_Mirroring::Vertical;
                if (mirroring == 3) m = NT_Mirroring::Horizontal;

                Emulator::instance()->get_ppu()->set_mirroring(m);
            }
            break;

            case 1: // CHR bank 0
                chr_low_switch();
                break;

            case 2: // CHR bank 1
                chr_high_switch();
                break;

            case 3: // PRG bank
                prg_switch();
                break;
            }

            register_ = {};
        }

        return true;
    }
    return false;
}

bool M001::on_ppu_read(address_t addr, byte_t& value)
{
    if (addr < 0x1000)
    {
        chr_l_.read(addr & 0x0FFF, value);
        return true;
    }
    else if (addr >= 0x1000 && addr < 0x2000)
    {
        chr_h_.read(addr & 0x0FFF, value);
        return true;
    }

    return false;
}

bool M001::on_ppu_write(address_t addr, byte_t value)
{
    if (addr < 0x2000)
    {
        // CHR RAM is always at bank 0.
        cart_.get_chr_bank(0)[addr] = value;
        return true;
    }

    return false;
}

BankView M001::get_bank(address_t addr) const
{
    if (addr >= 0x8000)
    {
        return  (addr < 0xC000) ? prg_l_ : prg_h_;
    }

    return {};
}

void M001::chr_low_switch()
{
    const bool mode_8kb = (control_ & 0x10) == 0;
    byte_t idx = register_.latch;

    if (mode_8kb)
        idx &= 0xFE;

    chr_l_.data_ = cart_.get_chr_bank(idx, 0x1000);

    if (mode_8kb)
        chr_h_.data_ = cart_.get_chr_bank(idx + 1, 0x1000);
}

void M001::chr_high_switch()
{
    const bool mode_8kb = (control_ & 0x10) == 0;
    byte_t idx = register_.latch;

    if (mode_8kb)
        return;

    chr_h_.data_ = cart_.get_chr_bank(idx, 0x1000);
}

void M001::prg_switch()
{
    byte_t mode = (control_ >> 2) & 0b11;
    byte_t idx = register_.latch & 0b1111;

    switch (mode)
    {
    case 0:
    case 1:
    {
        idx &= 0b1110;
        prg_l_.data_ = cart_.get_prg_bank(idx);
        prg_h_.data_ = cart_.get_prg_bank(idx + 1);
    }
    break;

    case 2:
    {
        prg_l_.data_ = cart_.get_prg_bank(0);
        prg_h_.data_ = cart_.get_prg_bank(idx);
    } 
    break;
   
    case 3:
    {
        prg_l_.data_ = cart_.get_prg_bank(idx);
        prg_h_.data_ = cart_.get_prg_bank(-1);
    }
    break;
    }
}