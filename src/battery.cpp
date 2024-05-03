#include "battery.h"

#include <filesystem>
#include <fstream>

using bifstream = std::basic_ifstream<byte_t>;
using bofstream = std::basic_ofstream<byte_t>;

NES_DEOPTIMIZE
Battery::Battery(stdfs::path filepath)
    : filepath_(std::move(filepath))
{
    if (!load_from(filepath_))
        memory_.fill(0_byte);
}

bool Battery::load_from(const stdfs::path& filepath)
{
    if (!stdfs::exists(filepath))
        return false;

    bifstream ifs(filepath);

    if (!ifs.is_open())
        return false;

    ifs.read(memory_.data(), memory_.size());

    return !ifs.bad();
}

bool Battery::save_now()
{
    stdfs::create_directories(filepath_.parent_path());

    bofstream ofs(filepath_);

    if (!ofs.is_open())
        return false;

    ofs.write(memory_.data(), memory_.size());

    return true;
}

bool Battery::write(address_t addr, byte_t value)
{
    if (addr >= 0x6000 && addr < 0x7FFF)
    {
        memory_[addr & 0x1FFF] = value;
        return true;
    }

    return false;
}

bool Battery::read(address_t addr, byte_t& value) const
{
    if (addr >= 0x6000 && addr < 0x7FFF)
    {
        value = memory_[addr & 0x1FFF];
        return true;
    }

    return false;
}

stdfs::path Battery::make_save_filepath(const stdfs::path& rom_filepath)
{
    stdfs::path filepath = rom_filepath.parent_path();
    filepath /= "saves";
    filepath /= rom_filepath.stem();
    filepath += ".battery";

    return filepath;
}
