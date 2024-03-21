#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <optional>

namespace ui
{
    void initialize();

    sf::Font const& get_font();
    

    struct RecentFile
    {
        std::wstring basename;
        std::wstring fullpath;
    };

    std::optional<RecentFile> get_recent_file(int idx);
    void push_recent_file(std::wstring_view path);
}