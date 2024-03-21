#include "global.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <ranges>

struct Globals
{
    bool init_ = false;
    sf::Font font_;

    std::array<ui::RecentFile, 5> recent_files_;

    static Globals instance_;
    static Globals& get() { return instance_; }
};

Globals Globals::instance_;

void ui::initialize()
{
    Globals& g = Globals::get();
    
    if (!g.init_)
    {
        g.init_ = true;
        g.font_.loadFromFile("data/Emulogic-zrEw.ttf");
    }
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