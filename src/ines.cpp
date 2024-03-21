#include "ines.h"

#include <iostream>
#include <fstream>

bool INESHeader::init(std::span<byte_t, 16> data)
{
    // Verify file begins with 'NES' tag.
    if (static_cast<char>(data[0]) != 'N'
        || static_cast<char>(data[1]) != 'E'
        || static_cast<char>(data[2]) != 'S'
        || data[3] != 0x1A) // end-of-file
        return false;

    struct : register_t<byte_t>
    {
        byte_t mirroring_ : 1;
        byte_t has_prg_ram_ : 1;
        byte_t has_trainer_ : 1;
        byte_t is_four_screen_ : 1;
        byte_t mapper_low_ : 4;
    } flag6;

    flag6.set(data[6]);

    struct : register_t<byte_t>
    {
        byte_t vs_unisystem_ : 1;
        byte_t playchoise_10_: 1;
        byte_t version_ : 2; // iNES 2.0 if it is equal to 2
        byte_t mapper_high_ : 4;
    } flag7;

    flag7.set(data[7]);

    struct : register_t<byte_t>
    {
        byte_t tv_system_ : 1; // 0 is ntsc, 1 is pal
        byte_t reserved : 7; // unused, must be all 0;
    } flag9;

    flag9.set(data[9]);

    if (flag9.reserved != 0)
        return false;

    prg_rom_size_ = data[4];
    chr_rom_size_ = data[5];


    mirroring_ = NT_Mirroring::None;
    if (!flag6.is_four_screen_)
    {
        mirroring_ = flag6.mirroring_ == 0 ? NT_Mirroring::Horizontal : NT_Mirroring::Vertical;
    }

    has_prg_ram_ = flag6.has_prg_ram_;
    has_trainer_ = flag6.has_trainer_;


    is_vs_unisystem_ = flag7.vs_unisystem_;
    is_playchoice_10_ = flag7.playchoise_10_;
    is_ines2_ = flag7.version_ == 2;

    mapper_ = flag7.mapper_high_ << 4 | (flag6.mapper_low_ & 0xF);

    prg_ram_size_ = data[8];

    is_ntsc_ = flag9.tv_system_ == 0;

    return true;
}

INESReader::INESReader() = default;
INESReader::~INESReader() = default;

bool INESReader::read_from_file(std::wstring_view filename)
{
    stream_ = std::make_unique<bifstream>(filename.data(), std::ios_base::binary);
    bifstream& ifs = *stream_;

    if (!ifs)
        return false;

    byte_t header[16];
    ifs.read(header, 16);

    if (!header_.init(header))
    {
        ifs.close();
        return false;
    }

    prg_rom_offset_ = 16 + (header_.has_trainer_ ? 512 : 0);
    chr_rom_offset_ = prg_rom_offset_ + static_cast<std::streamoff>(header_.prg_rom_size_ * 0x4000);

    return true;
}

void INESReader::read_prg_rom(int idx, std::span<byte_t, 0x4000> bank)
{
    if (!stream_)
        return;

    bifstream& ifs = *stream_;

    if (!ifs)
        return;

    ifs.seekg(prg_rom_offset_ + static_cast<std::streamoff>(idx * 0x4000));

    if (!ifs)
        return;

    ifs.read(bank.data(), 0x4000);
}

void INESReader::read_chr_rom(int idx, std::span<byte_t, 0x2000> bank)
{
    if (!stream_)
        return;

    bifstream& ifs = *stream_;

    if (!ifs)
        return;

    ifs.seekg(chr_rom_offset_ + static_cast<std::streamoff>(idx * 0x2000));

    if (!ifs)
        return;

    ifs.read(bank.data(), 0x2000);
}
