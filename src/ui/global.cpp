#include "global.h"

#include "imgui_cpu.h"
#include "imgui_ppu.h"

#include <algorithm>
#include <filesystem>
#include <ranges>

static ui::Globals* globals_ = nullptr;

ui::Globals& ui::Globals::get()
{
    return *globals_;
}

ui::Globals::Globals()
    : cpu_data_(new CPUData)
    , ppu_data_(new PPUData)
{
    font_.loadFromFile("data/Emulogic-zrEw.ttf");
}

void ui::initialize()
{
    static Globals globals;
    globals_ = &globals;
}

sf::Font const& ui::get_font()
{
    return Globals::get().font_;
}

std::optional<ui::RecentFile> ui::get_recent_file(int idx)
{
    if (idx < 0 || idx >= 5)
        return std::nullopt;

    RecentFile& rf = Globals::get().recent_files_[idx];
    if (rf.basename.empty())
        return std::nullopt;

    return rf;
}

void ui::push_recent_file(std::wstring_view path)
{
    auto& recent_files = Globals::get().recent_files_;

    auto fspath = std::filesystem::absolute(std::filesystem::path(path));
    auto basename = fspath.filename();

    auto it = std::ranges::find_if(recent_files, [&](RecentFile& rf) { return fspath.compare(rf.fullpath) == 0; });

    if (it == recent_files.end())
    {
        auto mid = recent_files.begin() + 4;
        std::rotate(recent_files.begin(), mid, recent_files.end());

        ui::RecentFile& rf = recent_files.front();
        rf.fullpath = std::filesystem::absolute(fspath).wstring();
        rf.basename = fspath.filename().wstring();
    }
    else if (it != recent_files.begin())
    {
        std::rotate(recent_files.begin(), it, it + 1);
    }
}