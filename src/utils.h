#include <fmt/format.h>

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