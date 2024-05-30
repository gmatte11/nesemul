#include "cartridge.h"

#include "ines.h"
#include "mapper.h"

Cartridge::Cartridge(byte_t ines_mapper_code)
{
    mapper_ = Mapper::create(ines_mapper_code);
}

address_t Cartridge::map_to_cpu_addr(address_t addr)
{
    if (addr >= 0x4020)
        return mapper_->map_to_cpu_addr(addr);
    return 0;
}

bool Cartridge::on_cpu_read(address_t addr, byte_t& value)
{
    if (addr >= 0x4020)
        return mapper_->on_cpu_read(addr, value);
    return false;
}

bool Cartridge::on_cpu_write(address_t addr, byte_t value)
{
    if (addr >= 0x4020)
        return mapper_->on_cpu_write(addr, value);
    return false;
}

bool Cartridge::on_ppu_read(address_t addr, byte_t& value)
{
    return mapper_->on_ppu_read(addr, value);
}

bool Cartridge::on_ppu_write(address_t addr, byte_t value)
{
    return mapper_->on_ppu_write(addr, value);
}

void Cartridge::load_roms(INESReader& reader)
{
    INESHeader& h = reader.header_;

    // 16 kb Program rom banks (PRG-ROM)
    if (h.prg_rom_size_ > 0)
    {
        std::array<byte_t, prg_bank_sz> bank;
        prg_rom_.resize(h.prg_rom_size_ * prg_bank_sz);

        for (int i = 0; i < h.prg_rom_size_; ++i)
        {
            reader.read_prg_rom(i, bank);
            std::memcpy(prg_rom_.data() + (i * prg_bank_sz), bank.data(), prg_bank_sz);
        }
    }

    // 8 kb Character rom banks (CHR-ROM)
    if (h.chr_rom_size_ > 0)
    {
        std::array<byte_t, chr_bank_sz> bank;
        chr_.resize(h.chr_rom_size_ * chr_bank_sz);

        for (int i = 0; i < h.chr_rom_size_; ++i)
        {
            reader.read_chr_rom(i, bank);
            std::memcpy(chr_.data() + (i * chr_bank_sz), bank.data(), chr_bank_sz);
        }
    }

    if (h.has_prg_ram_)
        battery_.reset(new Battery(Battery::make_save_filepath(reader.filepath_)));

    mapper_->post_load(*this);
}

std::pair<byte_t*, address_t> Cartridge::get_bank(address_t addr) const
{
    return mapper_->get_bank(addr);
}

const MemoryMap& Cartridge::get_mapped_prg() const
{
    return mapper_->get_cpu_mapped_prg_banks();
}

const MemoryMap& Cartridge::get_mapped_chr() const
{
    return mapper_->get_ppu_mapped_chr_banks();
}

byte_t* Cartridge::get_prg_bank(int idx) const
{
    const int count = static_cast<int>(prg_rom_.size() / prg_bank_sz);

    if (idx < 0)
        idx = count + idx;

    NES_ASSERT(idx >= 0 && idx < count);
    return const_cast<byte_t*>(prg_rom_.data()) + (idx * prg_bank_sz);
}

byte_t* Cartridge::get_chr_bank(int idx) const
{
    const int count = static_cast<int>(chr_.size() / chr_bank_sz);

    if (idx < 0)
        idx = count + idx;

    NES_ASSERT(idx >= 0 && idx < count);
    return const_cast<byte_t*>(chr_.data()) + (idx * chr_bank_sz);
}
