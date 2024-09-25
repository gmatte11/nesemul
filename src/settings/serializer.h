#pragma once

#include "types.h"

#include <nlohmann/json.hpp>
namespace nl = nlohmann;

#include <span>
#include <string_view>
#include <string>

class Serializer;

template <typename T>
concept Serializable = requires(T t)
{
    t.serialize(std::declval<Serializer&>());
};

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
    void process(std::string_view name, std::u8string& str);


    template <typename T, typename... Args>
    void process(std::string_view name, std::vector<T, Args...>& array)
    {
        if (is_writing())
        {
            if constexpr (Serializable<T>)
            {
                std::vector<nl::json> entries;
                entries.reserve(array.size());

                for (T& t : array)
                {
                    Serializer s;
                    t.serialize(s);
                    entries.emplace_back() = std::move(s.json_);
                }

                json_[name] = entries;
            }
            else
            {
                json_[name] = array;
            }
        }
        else
        {
            if constexpr (Serializable<T>)
            {
                std::vector<nl::json> entries = json_[name];
                array.clear();
                array.reserve(entries.size());

                for (auto& json_object : entries)
                {
                    Serializer s;
                    s.is_writing_ = false;
                    s.json_ = std::move(json_object);
                    array.emplace_back().serialize(s);
                }
            }
            else
            {
                json_[name].get_to(array);
            }
        }
    }

    void process(std::string_view name, Serializable auto& serializable)
    {
        if (is_writing())
        {
            Serializer s;
            serializable.serialize(s);
            json_[name] = std::move(s.json_);
        }
        else
        {
            Serializer s;
            s.is_writing_ = false;
            s.json_ = json_[name];
            serializable.serialize(s);
        }
    }

private:
    bool is_writing_ = true;
    nl::json json_;
};