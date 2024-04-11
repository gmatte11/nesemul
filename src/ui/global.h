#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <string>
#include <optional>

namespace ui
{
    struct CPUData;
    struct PPUData;

    struct RecentFile
    {
        std::wstring basename;
        std::wstring fullpath;
    };

    struct Globals
    {
        static Globals& get();

        Globals();

        bool init_ = false;
        sf::Font font_;

        std::array<ui::RecentFile, 5> recent_files_;

        std::unique_ptr<ui::CPUData> cpu_data_;
        std::unique_ptr<ui::PPUData> ppu_data_;
    };

    void initialize();

    sf::Font const &get_font();

    std::optional<RecentFile> get_recent_file(int idx);
    void push_recent_file(std::wstring_view path);
}