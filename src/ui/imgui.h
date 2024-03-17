#include <imgui.h>
#include <fmt/core.h>

namespace imgui
{
    using namespace ImGui;

    template <typename... Args>
    void TextFmt(fmt::format_string<Args...> format_str, Args&&... args)
    {
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), format_str, std::forward<Args>(args)...);
        TextUnformatted(buf.begin(), buf.end());
    }
}

namespace ui
{
    void imgui_mainmenu();
    void imgui_debugwindows();
}