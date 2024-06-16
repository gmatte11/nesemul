#include "imgui_ppu.h"

#include "emulator.h"
#include "bus.h"

#include "ui/global.h"
#include "ui/imgui.h"

#include <imgui-SFML.h>

#include <fmt/format.h>

void ui::PATData::update()
{
    Emulator& emulator = *Emulator::instance();
    const BUS& bus = *emulator.get_bus();
    const Cartridge* cart = bus.cart_;

    if (dirty_cart_check_ != cart)
    {
        dirty_cart_check_ = cart;

        chrbnk_combo_model_.clear();
        chrbnk_combo_model_.add_option("PPU Mapped");

        if (cart != nullptr)
        {
            for (size_t i = 0; i < std::size(cart->get_chr_banks()); ++i)
            {
                chrbnk_combo_model_.add_option(fmt::format("CHR bank {}", i));
            }
        }

        pat_idx_ = 0;
    }

    for (int i = 0; i < 8; ++i)
    {
        Palette p = ppu_read_palette(bus, i);
        sf::Texture &tex = pal_textures_[i];

        for (int j = 0; j < 4; ++j)
        {
            Color c = p.get(j);
            byte_t *pixel = reinterpret_cast<byte_t *>(&c);
            tex.update(pixel, 1, 1, j, 0);
        }
    }

    Palette palette = ppu_read_palette(bus, pal_idx_);

    if (cart != nullptr)
    {
        if (pat_idx_ == 0)
        {
            const MemoryMap& chr_map = cart->get_mapped_chr();
            if (chr_map[0].size_ == 0x1000 && chr_map[1].size_ == 0x1000)
            {
                ui::ppu_patterntable_texture(pat_textures_[0], {chr_map[0].mem_, 0x1000}, palette);
                ui::ppu_patterntable_texture(pat_textures_[1], {chr_map[1].mem_, 0x1000}, palette);
            }
            else
            {
                std::array<byte_t, 0x2000> chr_data;
                int idx = 0;
                for (const auto& bank : chr_map.map_)
                {
                    if (bank.mem_ != nullptr)
                        std::memcpy(chr_data.data() + idx, bank.mem_, bank.size_);
                    idx += bank.size_;
                }

                ui::ppu_patterntable_texture(pat_textures_[0], {chr_data.data(), 0x1000}, palette);
                ui::ppu_patterntable_texture(pat_textures_[1], {chr_data.data() + 0x1000, 0x1000}, palette);
            }
        }
        else
        {
            const byte_t* chr = cart->get_chr_bank(pat_idx_ - 1);
            ui::ppu_patterntable_texture(pat_textures_[0], {chr, 0x1000}, palette);
            ui::ppu_patterntable_texture(pat_textures_[1], {chr + 0x1000, 0x1000}, palette);
        }
    }
}

ui::PPUData& globals() { return *ui::Globals::get().ppu_data_; }

void debug_oam();
void debug_pat();
void debug_nam();

void ui::imgui_ppu_window()
{
    using namespace imgui;

    if (!globals().show_window_)
        return;

    if (Begin("PPU", &globals().show_window_))
    {
        if (BeginTabBar("-"))
        {
            if (BeginTabItem("OAM"))
            {
                debug_oam();
                EndTabItem();
            }

            if (BeginTabItem("PAT"))
            {
                debug_pat();
                EndTabItem();
            }

            if (BeginTabItem("NAM"))
            {
                debug_nam();
                EndTabItem();
            }

            EndTabBar();
        }
        End();
    }
}

void debug_oam()
{
    using namespace imgui;

    if (BeginChild("-"))
    {
        ui::OAMData& oam_data = globals().oam_data_;
        oam_data.update();

        auto entry = [&](int index)
        {
            const int x = (index % 8) * 8;
            const int y = (index / 8) * 8;

            sf::IntRect rect{ x, y, 8, 8 };
            sf::Sprite sfSprite(oam_data.texture_, rect);

            const OAMSprite& sprite = oam_data.sprites_[index];

            TextFmt("{:2}.", index);

            SameLine();
            imgui::Image(sfSprite, sf::Vector2f(16.f, 16.f));

            SameLine();
            TextFmt("{:02x} ({:3}, {:3}), {:02x}", sprite.tile_, sprite.x_, sprite.y_, sprite.att_.get());
        };

        if (BeginTable("-", 2))
        {
            for (int i = 0; i < 32; ++i)
            {
                TableNextRow();
                TableNextColumn();
                
                entry(i);

                TableNextColumn();

                entry(i + 32);
            }

            EndTable();
        }

        EndChild();
    }
}

void debug_pat()
{
    using namespace imgui;
    if (BeginChild("-"))
    {
        ui::PATData& pat_data = globals().pat_data_;
        pat_data.update();

        SeparatorText("Palettes");
        {
            std::string id;
            int selected = pat_data.pal_idx_;

            for (int i = 0; i < 8; ++i)
            {
                if (i == 0)
                    TextUnformatted("BG palettes: "); 

                if (i == 4)
                    TextUnformatted("FG palettes: ");

                SameLine();

                PushID(i);
                id = fmt::format("##Palette {}", i);

                ImVec2 cursor_pos = GetCursorPos();

                if (Selectable(id.c_str(), i == selected, 0, ImVec2{12 * 4, 12}))
                    selected = i;

                SetItemAllowOverlap();

                SetCursorPos({cursor_pos.x, cursor_pos.y});
                sf::Texture& tex = pat_data.pal_textures_[i];
                imgui::Image(tex, sf::Vector2f(12.f * 4.f, 12.f));

                PopID();
            }

            pat_data.pal_idx_ = selected;
        }

        SeparatorText("Pattern tables");
        {
            Combo("CHR bank", &pat_data.pat_idx_, pat_data.chrbnk_combo_model_.options_.data());

            sf::Texture& tex1 = pat_data.pat_textures_[0];
            imgui::Image(tex1, sf::Vector2f(256.f, 256.f));

            NewLine();

            sf::Texture& tex2 = pat_data.pat_textures_[1];
            imgui::Image(tex2, sf::Vector2f(256.f, 256.f));
        }

        EndChild();
    }
}

void debug_nam()
{
    using namespace imgui;

    if (BeginChild("-"))
    {
        ui::NAMData& nam_data = globals().nam_data_;

        nam_data.update();

        imgui::Image(nam_data.texture_, sf::Vector2f(512.f, 480.f));

        EndChild();
    }
}