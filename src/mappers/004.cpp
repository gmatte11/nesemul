#include "004.h"

#include "emulator.h"

NES_DEOPTIMIZE
M004::M004(Cartridge& cart)
    : cart_(cart)
{
    std::span<byte_t> empty_view;

    prg_map_[0] = { empty_view, 0x8000 };
    prg_map_[1] = { empty_view, 0xA000 };
    prg_map_[2] = { cart.get_prg_bank(-2, 0x2000), 0xC000 };
    prg_map_[3] = { cart.get_prg_bank(-1, 0x2000), 0xE000 };

    chr_map_[0] = { empty_view, 0x0000 };
    chr_map_[1] = { empty_view, 0x0400 };
    chr_map_[2] = { empty_view, 0x0800 };
    chr_map_[3] = { empty_view, 0x0C00 };
    chr_map_[4] = { empty_view, 0x1000 };
    chr_map_[5] = { empty_view, 0x1400 };
    chr_map_[6] = { empty_view, 0x1800 };
    chr_map_[7] = { empty_view, 0x1C00 };
}

bool M004::on_cpu_read(address_t addr, byte_t& value) 
{
    if (addr >= 0x6000 && addr < 0x8000)
    {
        if (!(cart_.battery_ && cart_.battery_->read(addr, value)))
            value = cart_.wram_[addr & 0x1FFF];
        return true;
    }

    if (addr >= 0x8000)
    {
        prg_map_.get_mapping(addr).read(addr & 0x1FFF, value);
        return true;
    }

    return false;
}

bool M004::on_cpu_write(address_t addr, byte_t value)
{
    if (addr >= 0x6000 && addr < 0x8000)
    {
        if (!(cart_.battery_ && cart_.battery_->write(addr, value)))
            cart_.wram_[addr & 0x1FFF] = value;
        return true;
    }

    if (addr >= 0x8000)
    {
        static constexpr address_t mode_mask = 0xE001;
        const address_t mode = mode_mask & addr;

        switch (mode)
        {
        case 0x8000:
            on_bank_select_(value);
            break;

        case 0x8001:
            on_bank_data_(value);
            break;

        case 0xA000:
            on_mirroring_(value);
            break;

        case 0xA001:
            on_prg_ram_protect_(value);
            break;

        case 0xC000:
            on_irq_latch_(value);
            break;

        case 0xC001:
            on_irq_reload_();
            break;

        case 0xE000:
            on_irq_disable_();
            break;

        case 0xE001:
            on_irq_enable_();
            break;
        }

        return true;
    }

    return false;
}

bool M004::on_ppu_read(address_t addr, byte_t& value)
{
    if (addr < 0x2000)
    {
        chr_map_.get_mapping(addr).read(addr & 0x03FF, value);
        return true;
    }

    return false;
}

bool M004::on_ppu_write(address_t addr, byte_t value)
{
    if (addr < 0x2000)
    {
        // CHR RAM is always at bank 0 ?
        cart_.get_chr_bank(0)[addr] = value;
        return true;
    }

    return false;
}

void M004::on_ppu_scanline(int scanline)
{
    if (irq_counter_ > 1)
        irq_counter_--;

    const bool raise_irq = (irq_enabled_ && irq_counter_ == 0);

    if (irq_reload_ || irq_counter_ == 0)
        irq_counter_ = irq_latch_;

    irq_reload_ = false;

    if (raise_irq)
        Emulator::instance()->get_cpu()->pull_irq();
}

void M004::on_bank_select_(byte_t value)
{
    Register recv;
    recv.set(value);

    if (register_.prg_mode_ != recv.prg_mode_)
    {
        std::swap(prg_map_[0].data_, prg_map_[2].data_);
    }

    register_.set(value);
}

void M004::on_bank_data_(byte_t value)
{
    switch (register_.bank_select_)
    {
    case 0b000:
        {
            const int idx = (register_.chr_mode_ == 0) ? 0 : 4;
            chr_map_[idx].data_ = cart_.get_chr_bank(value & 0xFE, 0x400);
            chr_map_[idx + 1].data_ = cart_.get_chr_bank((value & 0xFE) + 1, 0x400);
        }
        break;
    case 0b001:
        {
            const int idx = (register_.chr_mode_ == 0) ? 2 : 6;
            chr_map_[idx].data_ = cart_.get_chr_bank(value & 0xFE, 0x400);
            chr_map_[idx + 1].data_ = cart_.get_chr_bank((value & 0xFE) + 1, 0x400);
        }
        break;
    case 0b010:
        {
            const int idx = (register_.chr_mode_ == 0) ? 4 : 0;
            chr_map_[idx].data_ = cart_.get_chr_bank(value, 0x400);
        }
        break;
    case 0b011:
        {
            const int idx = (register_.chr_mode_ == 0) ? 5 : 1;
            chr_map_[idx].data_ = cart_.get_chr_bank(value, 0x400);
        }
        break;
    case 0b100:
        {
            const int idx = (register_.chr_mode_ == 0) ? 6 : 2;
            chr_map_[idx].data_ = cart_.get_chr_bank(value, 0x400);
        }
        break;
    case 0b101:
        {
            const int idx = (register_.chr_mode_ == 0) ? 7 : 3;
            chr_map_[idx].data_ = cart_.get_chr_bank(value, 0x400);
        }
        break;
    case 0b110:
        {
            const int idx = (register_.prg_mode_ == 0) ? 0 : 2;
            prg_map_[idx].data_ = cart_.get_prg_bank(value & 0x3F, 0x2000);
        }
        break;
    case 0b111:
        {
            prg_map_[1].data_ = cart_.get_prg_bank(value & 0x3F, 0x2000);
        }
        break;
    }
}

void M004::on_mirroring_(byte_t value)
{
    PPU* ppu = Emulator::instance()->get_ppu();

    if (ppu->get_mirroring() != NT_Mirroring::None)
    {
        ppu->set_mirroring((value & 0b1) == 0 ? NT_Mirroring::Vertical : NT_Mirroring::Horizontal);
    }
}

void M004::on_prg_ram_protect_(byte_t value)
{
    // Do nothing: might revise for MMC6
}

void M004::on_irq_latch_(byte_t value)
{
    irq_latch_ = value;
}

void M004::on_irq_reload_()
{
    irq_reload_ = true;
    irq_counter_ = 0xFF;
}

void M004::on_irq_disable_()
{
    irq_enabled_ = false;
    // acknowledge pending interrupts?
}

void M004::on_irq_enable_()
{
    irq_enabled_ = true;
}
