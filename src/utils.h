#pragma once

#include <fmt/format.h>

#include <array>
#include <concepts>
#include <span>
#include <utility>

#define NES_BREAKPOINT __debugbreak()
#define NES_DEOPTIMIZE __pragma(optimize("",off))

#define NES_ASSERT(X) { if(!(X)) { __debugbreak(); } }

#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

#define PARENS ()

#define FOR_EACH_AGAIN() FOR_EACH_HELPER
#define FOR_EACH_ARGS_AGAIN() FOR_EACH_ARGS_HELPER
#define FOR_EACH_HELPER(macro, a, ...) macro(a) __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_ARGS_HELPER(macro, a, ...) macro(a) __VA_OPT__(, FOR_EACH_ARGS_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_ARGS(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_ARGS_HELPER(macro, __VA_ARGS__)))

#define STRING_IMPL(...) #__VA_ARGS__
#define STRINGIFY(...) __VA_OPT__(STRING_IMPL (__VA_ARGS__))


struct StringBuilder
{
    fmt::memory_buffer buf;

    auto to_string() const { return fmt::to_string(buf); }

    auto appender() { return std::back_inserter(buf); }

    StringBuilder& append(auto rng)
    {
        buf.append(rng);
        return *this;
    }

    template <typename... Args>
    StringBuilder& append_fmt(fmt::format_string<Args...> fmt_string, Args&&... args)
    {
        fmt::format_to(appender(), fmt_string, std::forward<decltype(args)>(args)...);
        return *this;
    }
};

template <typename EnumT, typename UnderlyingT>
struct NamedEnum
{
    NamedEnum() = default;
    NamedEnum(const NamedEnum&) = default;
    explicit NamedEnum(UnderlyingT value) : value_(value) {}

    NamedEnum& operator=(const NamedEnum&) = default;

    const std::string_view to_string() const { return EnumT::dict_[value_]; }

    bool operator==(NamedEnum rhs) const { return value_ == rhs.value_; }
    auto operator<=>(NamedEnum rhs) const { return value_ <=> rhs.value_; }

    operator UnderlyingT() const { return value_; }

    static EnumT from_value(std::integral auto value) { return EnumT(static_cast<UnderlyingT>(value)); }

    static EnumT from_string(std::string_view name, bool caseInsencitive = false)
    {
        auto char_proj = caseInsencitive ? std::identity::operator()<char> : [](char c) { return std::tolower(c); };

        for (size_t i = 0; i < EnumT::dict_.size(); ++i)
        {
            const std::string_view sv = EnumT::dict_[i];
            if (std::ranges::equal(sv, name, std::equal_to<>(), char_proj, char_proj))
                return EnumT(static_cast<int>(i));
        }

        return EnumT::Count_;
    }

    static std::span<const std::string_view> get_strings() { return EnumT::dict_; }

    template <typename... Names>
    static constexpr auto make_dictionary(Names... names) 
    {
        using namespace std::literals;
        return std::array<const std::string_view, sizeof...(Names) + 1> { std::string_view{names}..., "Invalid"sv };
    }

    UnderlyingT value_ = EnumT::First_;
};

#define NAMED_ENUM_IMPL(Typename, UnderlyingType, FirstEntry, ...)                                        \
    struct Typename : NamedEnum<Typename, UnderlyingType>                                                 \
    {                                                                                                     \
        using NamedEnum::NamedEnum;                                                                       \
        enum InternalEnum : UnderlyingType                                                                \
        {                                                                                                 \
            First_ = UnderlyingType{},                                                                    \
            FirstEntry = First_,                                                                          \
            __VA_ARGS__,                                                                                  \
            Count_,                                                                                       \
            Last_ = Count_ - 1                                                                            \
        };                                                                                                \
        static constexpr auto dict_ = make_dictionary(FOR_EACH_ARGS(STRINGIFY, FirstEntry, __VA_ARGS__)); \
    }

#define NAMED_ENUM(Typename, ...) NAMED_ENUM_IMPL(Typename, int, __VA_ARGS__)
#define NAMED_TYPED_ENUM(Typename, UnderlyingType, ...) NAMED_ENUM_IMPL(Typename, UnderlyingType, __VA_ARGS__)