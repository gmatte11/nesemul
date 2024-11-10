#include "cartridge.h"

#include "ines.h"
#include "mappers/mapper.h"

Cartridge::Cartridge()
{
}

Cartridge::~Cartridge()
{
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

void Cartridge::on_ppu_scanline(int scanline)
{
    mapper_->on_ppu_scanline(scanline);
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
        chr_rom_.resize(h.chr_rom_size_ * chr_bank_sz);

        for (int i = 0; i < h.chr_rom_size_; ++i)
        {
            reader.read_chr_rom(i, bank);
            std::memcpy(chr_rom_.data() + (i * chr_bank_sz), bank.data(), chr_bank_sz);
        }
    }
    else
    {
        // If no rom banks, expand to one bank of chr ram.
        chr_rom_.resize(chr_bank_sz);
    }

    if (h.has_prg_ram_)
        battery_.reset(new Battery(Battery::make_save_filepath(reader.filepath_)));

    mapper_.reset(Mapper::create(h.mapper_, *this));
}

BankView Cartridge::get_cpu_mapped_bank(address_t addr) const
{
    return mapper_->get_cpu_mapped_bank(addr);
}

const MemoryMap& Cartridge::get_mapped_prg() const
{
    return mapper_->get_cpu_mapped_prg_banks();
}

const MemoryMap& Cartridge::get_mapped_chr() const
{
    return mapper_->get_ppu_mapped_chr_banks();
}

size_t Cartridge::calc_prg_offset(int idx, size_t bank_sz /*= prg_bank_sz*/) const
{
    const int count = int_cast<int>(prg_rom_.size() / bank_sz);

    if (idx < 0)
        idx = count + idx;

    return idx * bank_sz;
}

size_t Cartridge::calc_chr_offset(int idx, size_t bank_sz /*= chr_bank_sz*/) const
{
    const int count = int_cast<int>(chr_rom_.size() / bank_sz);

    if (idx < 0)
        idx = count + idx;

    return idx * bank_sz;
}

std::span<byte_t> Cartridge::get_prg_bank(int idx, size_t bank_sz /*= prg_bank_sz*/)
{
    const size_t offset = calc_prg_offset(idx, bank_sz);
    NES_ASSERT((offset + bank_sz) <= prg_rom_.size());
    return { prg_rom_.data() + offset, bank_sz };
}

std::span<const byte_t> Cartridge::get_prg_bank(int idx, size_t bank_sz) const
{
    const size_t offset = calc_prg_offset(idx, bank_sz);
    NES_ASSERT((offset + bank_sz) <= prg_rom_.size());
    return { prg_rom_.data() + offset, bank_sz };
}

std::span<byte_t> Cartridge::get_chr_bank(int idx, size_t bank_sz /*= chr_bank_sz*/)
{
    const size_t offset = calc_chr_offset(idx, bank_sz);
    NES_ASSERT((offset + bank_sz) <= chr_rom_.size());
    return { chr_rom_.data() + offset, bank_sz };
}

std::span<const byte_t> Cartridge::get_chr_bank(int idx, size_t bank_sz) const
{
    const size_t offset = calc_chr_offset(idx, bank_sz);
    NES_ASSERT((offset + bank_sz) <= chr_rom_.size());
    return { chr_rom_.data() + offset, bank_sz };
}
