#pragma once

#include "clock.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <string>
#include <optional>

class Serializer;

namespace ui
{
    struct CPUData;
    struct PPUData;

    struct RecentFile
    {
        void serialize(Serializer& serializer);

        std::wstring basename_;
        std::u8string fullpath_;
    };

    struct Globals
    {
        static Globals& get();

        Globals();

        bool init_ = false;
        sf::Font font_;

        std::vector<ui::RecentFile> recent_files_;

        std::unique_ptr<ui::CPUData> cpu_data_;
        std::unique_ptr<ui::PPUData> ppu_data_;

        FPSCounter fps_counter_;
    };

    void initialize();
    void save_settings();

    sf::Font const &get_font();

    std::optional<RecentFile> get_recent_file(int idx);
    void push_recent_file(std::wstring_view path);
}