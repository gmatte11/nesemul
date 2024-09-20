#include "serializer.h"

std::optional<Serializer> Serializer::from_string(std::string_view sv)
{
    if (!nl::json::accept(sv))
        return std::nullopt;

    Serializer s;
    s.is_writing_ = false;
    s.json_ = nl::json::parse(sv);
    return s;
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