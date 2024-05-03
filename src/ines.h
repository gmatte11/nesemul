#pragma once

#include "types.h"

#include <iosfwd>
#include <span>
#include <string_view>
#include <memory>

using bifstream = std::basic_ifstream<byte_t>;

struct INESHeader
{
    byte_t mapper_;

    byte_t prg_rom_size_;
    byte_t chr_rom_size_;
    byte_t prg_ram_size_;

    NT_Mirroring mirroring_;

    bool is_ines2_;
    bool is_ntsc_;
    bool is_vs_unisystem_;
    bool is_playchoice_10_;
    bool has_prg_ram_;
    bool has_trainer_;

    bool init(std::span<byte_t, 16> data);
};

class INESReader
{
public:
    INESReader();
    ~INESReader();

    bool read_from_file(std::wstring_view filename);

    void read_prg_rom(int idx, std::span<byte_t, 0x4000> bank);
    void read_chr_rom(int idx, std::span<byte_t, 0x2000> bank);

    INESHeader header_;
    std::wstring filepath_;

private:
    std::unique_ptr<bifstream> stream_;
    std::streampos prg_rom_offset_;
    std::streampos chr_rom_offset_;
};