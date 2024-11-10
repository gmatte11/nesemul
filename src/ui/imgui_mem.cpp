#include "imgui_mem.h"

#include "types.h"

#include "emulator.h"

#include "ui/imgui.h"

void ui::imgui_mem_view()
{
    using namespace imgui;

    std::span<byte_t> ram = {Emulator::instance()->get_bus()->ram_.data(), 0x800};
    const float textHeight = GetTextLineHeightWithSpacing();

    constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
    if (BeginTable("hex", 18, table_flags, ImVec2(0, textHeight * 32)))
    {
        constexpr ImGuiTableColumnFlags col_flags = 0;

        TableSetupColumn("Address", 0, 0.f, col_flags);

        constexpr std::array<const char[3], 16> byte_col_labels =
            {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F"};

        for (const char* label : byte_col_labels)
            TableSetupColumn(label, 0, 0.f, col_flags);

        TableSetupColumn("ASCII", 0, 0.f, col_flags);

        TableSetupScrollFreeze(0, 1);
        TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(int_cast<int>(ram.size()) / 16, textHeight);

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                int col_addr = row * 16;

                TableNextRow();

                TableNextColumn();
                TextFmt("{:04X}:", col_addr);

                char ascii[16];

                for (int byte_idx = 0; byte_idx < 16; ++byte_idx)
                {
                    const int idx = col_addr + byte_idx;
                    byte_t byte = ram[idx];

                    if (byte < 128 && std::isprint(byte))
                        ascii[byte_idx] = static_cast<char>(byte);
                    else
                        ascii[byte_idx] = '.';

                    TableNextColumn();
                    PushID(fmt::format("{}", idx).c_str());
                    std::string byte_str = fmt::format("{:02X}", byte);

                    if (BeginPopup("edit"))
                    {
                        byte_t one = 1;
                        byte_t sixteen = 16;
                        if (InputScalar("value", ImGuiDataType_U8, &byte, &one, &sixteen, "%02X", ImGuiInputTextFlags_EnterReturnsTrue))
                            ram[idx] = byte;
                        EndPopup();
                    }

                    if (Selectable(byte_str.c_str(), false))
                        OpenPopup("edit");

                    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip) && !IsPopupOpen("edit"))
                    {
                        BeginTooltip();
                        TextFmt("Decimal: {}", byte);
                        EndTooltip();
                    }

                    PopID();
                }

                TableNextColumn();
                TextUnformatted(ascii, ascii + 16);
            }
        }

        EndTable();
    }
}