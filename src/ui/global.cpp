#include "global.h"

#include "imgui.h"
#include "imgui_cpu.h"
#include "imgui_ppu.h"

#include "settings/serializer.h"

#include <algorithm>
#include <filesystem>
#include <ranges>

static constexpr std::wstring_view settings_filename = L"data/settings.cfg";
static constexpr size_t recent_file_max = 5;

void serialize_ui_settings(Serializer& serializer, ui::Globals& globals)
{
    serializer.process("recent_files", globals.recent_files_);
    
    if (globals.recent_files_.size() > recent_file_max)
        globals.recent_files_.resize(recent_file_max);
}

void ui::RecentFile::serialize(Serializer& serializer)
{
    serializer.process("fullpath", fullpath_);

    if (!serializer.is_writing())
    {
        auto fspath = std::filesystem::absolute(std::filesystem::path(fullpath_));
        basename_ = fspath.filename();
    }
}

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

    auto opt = Serializer::open_read(settings_filename);
    if (opt)
    {
        serialize_ui_settings(opt.value(), *this);
    }
}

void ui::initialize()
{
    static Globals globals;
    globals_ = &globals;
}

void ui::save_settings()
{
    Serializer serializer;
    serialize_ui_settings(serializer, Globals::get());
    serializer.write_file(settings_filename, true);
}

sf::Font const& ui::get_font()
{
    return Globals::get().font_;
}

std::optional<ui::RecentFile> ui::get_recent_file(int idx)
{
    auto& recent_files = Globals::get().recent_files_;
    if (idx < 0 || idx >= recent_files.size())
        return std::nullopt;

    RecentFile& rf = recent_files[idx];
    if (rf.basename_.empty())
        return std::nullopt;

    return rf;
}

void ui::push_recent_file(std::wstring_view path)
{
    auto& recent_files = Globals::get().recent_files_;

    auto fspath = std::filesystem::absolute(std::filesystem::path(path));
    auto basename = fspath.filename();

    auto it = std::ranges::find_if(recent_files, [&](RecentFile& rf) { return fspath.compare(rf.fullpath_) == 0; });

    if (it == recent_files.end())
    {
        if (recent_files.size() >= recent_file_max)
        {
            auto mid = recent_files.begin() + (recent_file_max - 1);
            std::rotate(recent_files.begin(), mid, recent_files.end());

            ui::RecentFile& rf = recent_files.front();
            rf.fullpath_ = std::filesystem::absolute(fspath).u8string();
            rf.basename_ = fspath.filename().wstring();

            recent_files.resize(recent_file_max);
        }
        else
        {
            ui::RecentFile rf;
            rf.fullpath_ = std::filesystem::absolute(fspath).u8string();
            rf.basename_ = fspath.filename().wstring();
            recent_files.insert(recent_files.begin(), std::move(rf));
        }
    }
    else if (it != recent_files.begin())
    {
        std::rotate(recent_files.begin(), it, it + 1);
    }
}