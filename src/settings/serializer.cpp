#include "serializer.h"

#include <codecvt>
#include <fstream>
#include <filesystem>
#include <locale>

namespace stdfs = std::filesystem;

std::optional<Serializer> Serializer::open_read(std::wstring_view filename)
{
    std::ifstream ifs;
    ifs.open(stdfs::absolute(filename));
    if (ifs)
    {
        if (ifs.peek() == std::ios::traits_type::eof())
            return std::nullopt;

        Serializer s;
        s.is_writing_ = false;
        s.json_ = nl::json::parse(ifs);
        return {s};
    }
    
    return std::nullopt;
}

std::optional<Serializer> Serializer::from_string(std::string_view sv)
{
    if (!nl::json::accept(sv))
        return std::nullopt;

    Serializer s;
    s.is_writing_ = false;
    s.json_ = nl::json::parse(sv);
    return s;
}

bool Serializer::write_file(std::wstring_view filename, bool prettyfied)
{
    std::ofstream ofs;
    ofs.open(stdfs::absolute(filename), std::ios::out | std::ios::trunc);
    if (ofs)
    {
        ofs << to_string(prettyfied);
        return true;
    }

    return false;
}

std::string Serializer::to_string(bool prettyfied /*= false*/) const
{
    return json_.dump(prettyfied ? 4 : -1);
}

void Serializer::process(std::string_view name, int8_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, uint8_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, int16_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, uint16_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, int32_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, uint32_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, int64_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, uint64_t& value)
{
    if (is_writing())
    {
        json_.update({{name, value}});
    }
    else
    {
        value = json_[name];
    }
}

void Serializer::process(std::string_view name, std::string& str)
{
    if (is_writing())
    {
        json_.update({{name, str}});
    }
    else
    {
        str = json_[name];
    }
}

void Serializer::process(std::string_view name, std::u8string& str)
{
    if (is_writing())
    {
        json_.update({{name, (const char*)str.data()}});
    }
    else
    {
        str = (const char8_t*)json_[name].get<nl::json::string_t>().data();
    }
}