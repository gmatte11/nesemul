#include "001.h"
#include "cartridge.h"
#include "emulator.h"

bool M001::on_cpu_read(address_t addr, byte_t& value) 
{
    if (addr >= 0x6000 && addr < 0x8000)
    {
        value = cart_->wram_[addr & 0x1FFF];
        return true;
    }

    if (addr >= 0x8000 && addr < 0xC000)
    {
        value = *(prg_l_ + (addr & 0x3FFF));
        return true;
    }

    if (addr >= 0xC000)
    {
        value = *(prg_h_ + (addr & 0x3FFF));
        return true;
    }

    return false;
}

bool M001::on_cpu_write(address_t addr, byte_t value)
{
    if (addr >= 0x6000 && addr < 0x8000)
    {
        cart_->wram_[addr & 0x1FFF] = value;
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
                prg_h_ = cart_->prg_rom_.back().data();

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
    if (addr < 0x1000 && chr_l_ != nullptr)
    {
        value = *(chr_l_ + addr);
        return true;
    }
    else if (addr >= 0x1000 && addr < 0x2000 && chr_h_ != nullptr)
    {
        value = *(chr_h_ + (addr & 0xFFF));
        return true;
    }

    return false;
}

bool M001::on_ppu_write(address_t addr, byte_t value)
{
    if (addr < 0x2000)
    {
        // CHR RAM is always at bank 0.
        cart_->chr_.front()[addr] = value;
        return true;
    }

    return false;
}

std::pair<byte_t*, address_t> M001::get_bank(address_t addr) const
{
    if (addr >= 0x8000)
    {
        return  std::pair((addr < 0xC000) ? prg_l_ : prg_h_, static_cast<address_t>(addr & 0x3FFF));
    }

    return std::pair(nullptr, 0_addr);
}

void M001::chr_low_switch()
{
    const bool mode_8kb = (control_ & 0x10) == 0;
    byte_t idx = register_.latch;

    if (mode_8kb)
        idx &= 0xFE;

    int chr_idx = idx >> 1;
    int offset = (idx & 0x1) ? 0x1000 : 0;

    NES_ASSERT(chr_idx < cart_->chr_.size());
    chr_l_ = cart_->chr_[chr_idx].data() + offset;

    if (mode_8kb)
        chr_h_ = chr_l_ + 0x1000;
}

void M001::chr_high_switch()
{
    const bool mode_8kb = (control_ & 0x10) == 0;
    byte_t idx = register_.latch;

    if (mode_8kb)
        return;

    int chr_idx = idx >> 1;
    int offset = (idx & 0x1) ? 0x1000 : 0;

    NES_ASSERT(chr_idx < cart_->chr_.size());
    chr_h_ = cart_->chr_[chr_idx].data() + offset;
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
        NES_ASSERT(idx + 1 < cart_->prg_rom_.size());
        prg_l_ = cart_->prg_rom_[idx].data();
        prg_h_ = cart_->prg_rom_[idx + 1].data();
    }
    break;

    case 2:
    {
        NES_ASSERT(idx < cart_->prg_rom_.size());
        prg_l_ = cart_->prg_rom_.front().data();
        prg_h_ = cart_->prg_rom_[idx].data();
    } 
    break;
   
    case 3:
    {
        NES_ASSERT(idx < cart_->prg_rom_.size());
        prg_l_ = cart_->prg_rom_[idx].data();
        prg_h_ = cart_->prg_rom_.back().data();
    }
    break;
    }
}