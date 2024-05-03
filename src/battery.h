#pragma once

#include <array>
#include <filesystem>
#include <string_view>

#include "types.h"

namespace stdfs = std::filesystem;

class Battery
{
  public:
    Battery(stdfs::path filepath);

    bool load_from(const stdfs::path& filepath);
    bool save_now();

    bool write(address_t addr, byte_t value);
    bool read(address_t addr, byte_t& value) const;

    static stdfs::path make_save_filepath(const stdfs::path& rom_filepath);

private:
    std::array<byte_t, 0x2000> memory_;
    stdfs::path filepath_;
};