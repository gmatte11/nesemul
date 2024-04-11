#pragma once

#include "ui/ppu_utils.h"

#include <SFML/Graphics.hpp>

#include <array>

namespace ui
{
    struct ComboBoxModel
    {
        void clear() { options_.clear(); }

        void add_option(std::string_view sv)
        {
            constexpr char terminator[] = {'\0', '\0'};

            if (options_.size() > 0)
                options_.pop_back();

            options_.append_range(sv);
            options_.append_range(terminator);
        }

        explicit operator bool() const { return !options_.empty(); }

        std::vector<char> options_;
    };

    struct OAMData
    {
        OAMData() { texture_.create(64, 64); }

        void update() { ui::ppu_oam_texture(texture_, sprites_); }

        std::array<OAMSprite, 64> sprites_;
        sf::Texture texture_;
    };

    struct PATData
    {
        PATData()
        {
            for (sf::Texture &tex : pal_textures_)
                tex.create(4, 1);

            for (sf::Texture &tex : pat_textures_)
                tex.create(128, 128);
        }

        void update();

        std::array<sf::Texture, 8> pal_textures_;
        std::array<sf::Texture, 2> pat_textures_;
        const void *dirty_cart_check_ = nullptr;
        ComboBoxModel chrbnk_combo_model_;
        int pal_idx_ = 0;
        int pat_idx_ = 0;
    };

    struct NAMData
    {
        NAMData() { texture_.create(512, 480); }

        void update() { ui::ppu_nametable_texture(texture_); }

        sf::Texture texture_;
    };

    struct PPUData
    {
        OAMData oam_data_;
        PATData pat_data_;
        NAMData nam_data_;

        bool show_window_ = false;
    };
}

namespace ui
{
    void imgui_ppu_window();   
}
