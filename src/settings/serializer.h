#pragma once

#include "types.h"

#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include <span>
#include <string_view>

enum class SerialType
{
    RAW_DATA,
    INT_8,
    INT_16,
    INT_32,
    INT_64,
    STRING
};

class Serializer
{
public:
    Serializer() = default;

    bool is_writing() const { return is_writing_; }

    static std::optional<Serializer> open_read(std::wstring_view filename);
    static std::optional<Serializer> from_string(std::string_view sv);

    bool write_file(std::wstring_view filename, bool prettyfied = false);

    std::string to_string(bool prettyfied = false) const;

    void process(std::string_view name, int8_t& value);
    void process(std::string_view name, uint8_t& value);
    void process(std::string_view name, int16_t& value);
    void process(std::string_view name, uint16_t& value);
    void process(std::string_view name, int32_t& value);
    void process(std::string_view name, uint32_t& value);
    void process(std::string_view name, int64_t& value);
    void process(std::string_view name, uint64_t& value);

    void process(std::string_view name, std::string& str);

    template <std::ranges::common_range Rng>
    void process(std::string_view name, Rng& array)
    {
        if (is_writing())
        {
            json_.update({{name, nl::json(array)}});
        }
        else
        {
            json_[name].get_to(array);
        }
    }

private:
    bool is_writing_ = true;
    nl::json json_;
};