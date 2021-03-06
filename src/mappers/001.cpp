#include "001.h"
#include "cartridge.h"

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
        bool full = register_ & 0b1;
        (register_ >>= 1) |= ((0x1 & value) << 4);
        bool reset = (0x80 & value);

        if (full || reset)
        {
            int op = (addr >> 13) & 0b11;
            switch (op)
            {
            case 0: // control
            {
                if (full)
                    control_ = register_;
                else
                    control_ |= 0xC;
            }
            break;

            case 1: // CHR bank 0
                chr_switch(true);
                break;

            case 2: // CHR bank 1
                chr_switch(false);
                break;

            case 3: // PRG bank
            {
                if (reset)
                    prg_h_ = cart_->prg_rom_.back().data();
                else
                    prg_switch();
            }
            break;
            }

            register_ = 0b10000;
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
    if (addr < 0x2000 && chr_l_ == cart_->chr_ram_.data())
    {
        cart_->chr_ram_[addr & 0x1FFF] = value;
        return true;
    }

    return false;
}

std::pair<byte_t*, address_t> M001::get_bank(address_t addr) const
{
    if (addr >= 0x8000)
    {
        return  std::make_pair((addr < 0xC000) ? prg_l_ : prg_h_, addr & 0x3FFF);
    }

    return std::make_pair(nullptr, 0u);
}

void M001::chr_switch(bool low)
{
    bool mod_4kb = control_ & 0x10;

    if (mod_4kb)
    {
        int idx = register_ & 0x1FFF;
        address_t offset = (idx & 0x01) ? 0x1000 : 0;

        byte_t *& chr = (low) ? chr_l_ : chr_h_;
        chr = cart_->chr_rom_[idx >> 1].data() + offset;
    }
    else if (low)
    {
        if (register_ < cart_->chr_rom_.size())
        {
            chr_l_ = cart_->chr_rom_[register_ >> 1].data();
            chr_h_ = chr_l_ + 0x1000;
        }
    }
}

void M001::prg_switch()
{
    byte_t mode = (control_ >> 2) & 0b11;
    byte_t idx = register_ & 0b1111;

    switch (mode)
    {
    case 0:
    case 1:
    {
        idx = idx >> 1;
        prg_l_ = cart_->prg_rom_[idx].data();
        prg_h_ = (cart_->prg_rom_.size() > idx) ? cart_->prg_rom_[idx + 1].data() : prg_l_;
    }
    break;

    case 2:
    {
        prg_l_ = cart_->prg_rom_.front().data();
        prg_h_ = cart_->prg_rom_[idx].data();
    } 
    break;
   
    case 3:
    {
        prg_h_ = cart_->prg_rom_.back().data();
        prg_l_ = cart_->prg_rom_[idx].data();
    }
    break;
    }
}