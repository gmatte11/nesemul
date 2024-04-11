#include "ui/imgui_debugger.h"
#include "ui/imgui.h"

#include "emulator.h"
#include "cartridge.h"
#include "cpu.h"
#include "ops.h"

#include "types.h"
#include "utils.h"

struct PrgModel
{
    using PrgBank = Disassembler::PrgBank;
    using Op = Disassembler::Op;

    void clear()
    {
        banks_.fill(nullptr);
        cart_check = nullptr;
    }

    void reset(const BUS& bus, int prg_idx)
    {
        if (bus.cart_ == nullptr)
        {
            clear();
            return;
        }

        Disassembler& disassembler = Emulator::instance()->disassembler_;

        const PrgBank* bank1 = nullptr;
        const PrgBank* bank2 = nullptr;

        if (prg_idx == 0)
        {
            std::tie(bank1, bank2) = disassembler.get_mapped_banks(bus);
        }
        else
        {
            const int disassembler_bank_idx = (prg_idx - 1) * 2; 

            auto banks = disassembler.get_banks();
            bank1 = &banks[disassembler_bank_idx];
            bank2 = &banks[disassembler_bank_idx + 1];
        }

        banks_[0] = bank1;
        banks_[1] = bank2;

        if (banks_[0] != nullptr)  
            sizes_[0] = static_cast<int>(banks_[0]->ops_.size());

        if (banks_[1] != nullptr) 
            sizes_[1] = static_cast<int>(banks_[1]->ops_.size());
    }

    int size() const
    {
        return sizes_[0] + sizes_[1];
    }

    Op get_op(int idx)
    {
        NES_ASSERT(idx >= 0 && idx < sizes_[0] + sizes_[1]);
        if (idx < sizes_[0])
            return banks_[0]->ops_[idx];

        return banks_[1]->ops_[idx - sizes_[0]];
    }

    int get_idx(address_t addr)
    {
        auto find_idx = [](std::span<const Op> ops, address_t addr)
        {
            for (size_t i = 0; i < ops.size(); ++i)
            {
                if (ops[i].addr_ == addr)
                    return static_cast<int>(i);
            }

            return -1;
        };

        if (addr < 0x4000 && banks_[0] != nullptr)
            return find_idx(banks_[0]->ops_, addr);

        if (addr >= 0x4000 && banks_[1] != nullptr)
            return find_idx(banks_[1]->ops_, addr - 0x4000);
            
        return -1;
    }

    std::array<const PrgBank*, 2> banks_ = {};
    std::array<int, 2> sizes_= {};

    void* cart_check = nullptr;
};

void ui::imgui_debugger()
{
    using namespace imgui;
    Emulator& emulator = *Emulator::instance();
    BUS& bus = *emulator.get_bus();
    CPU& cpu = *emulator.get_cpu();
    CPU_State const& cpu_state = cpu.get_state();

    static PrgModel model;
    model.reset(bus, 0);

    address_t prg_offset = bus.map_cpu_addr(0x8000);
    address_t program_counter = bus.map_cpu_addr(cpu_state.program_counter_);

    int sticky_idx = model.get_idx(program_counter - prg_offset);

    const float textHeight = GetTextLineHeightWithSpacing();

    if (BeginTable("layout_columns", 2))
    {
        TableNextRow();
        TableNextColumn();

        {
            SeparatorText("Instructions");
            
            static ImGuiTabBarFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
            if (BeginTable("disassembly", 5, flags, ImVec2(0.f, textHeight * 16)))
            {
                constexpr ImGuiTableColumnFlags op_flags = ImGuiTableColumnFlags_NoResize;

                TableSetupColumn("Address", 0, 0.f);
                TableSetupColumn("opcode", op_flags, 0.f);
                TableSetupColumn("operand1", op_flags, 0.f);
                TableSetupColumn("operand2", op_flags, 0.f);
                TableSetupColumn("Assembly", 0, 0.f);

                StringBuilder sb;

                const int instrCount = static_cast<int>(model.size());
                ImGuiListClipper clipper;
                clipper.Begin(instrCount, textHeight);

                if (sticky_idx >= 0)
                    clipper.IncludeItemsByIndex(std::max(0, sticky_idx - 5), std::min(sticky_idx + 5, instrCount));

                while (clipper.Step())
                {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
                    {
                        PrgModel::Op op = model.get_op(row);
                        const ops::metadata& meta = ops::opcode_data(op.opcode_);

                        const bool is_at_pc = sticky_idx >= 0 && sticky_idx == row;

                        TableNextRow();

                        TableNextColumn();
                        const address_t mapped_addr = bus.map_cpu_addr(op.addr_ + prg_offset);
                        TextFmt("{}{:04X}", is_at_pc ? '*' : ' ', mapped_addr);

                        if (is_at_pc)
                            SetScrollHereY();

                        TableNextColumn();
                        TextFmt("{:02X}", op.opcode_);

                        TableNextColumn();
                        if (meta.get_size() >= 2)
                            TextFmt("{:02X}", op.operand1_);
                        else
                            TextUnformatted("  ");

                        TableNextColumn();
                        if (meta.get_size() == 3)
                            TextFmt("{:02X}", op.operand2_);
                        else
                            TextUnformatted("  ");

                        TableNextColumn();
                        sb.buf.clear();
                        emulator.disassembler_.asm_str(sb, op);
                        TextUnformatted(sb.buf.data(), sb.buf.data() + sb.buf.size());
                    }
                }

                EndTable();
            }
        }

        TableNextColumn();
        {
            SeparatorText("CPU State");

            TextFmt("Flags: {}{}xx{}{}{}{}\n",
                    (cpu_state.status_ & CPU_State::kNegative) ? 'N' : '-',
                    (cpu_state.status_ & CPU_State::kOverflow) ? 'O' : '-',
                    (cpu_state.status_ & CPU_State::kDecimal) ? 'D' : '-',
                    (cpu_state.status_ & CPU_State::kIntDisable) ? 'I' : '-',
                    (cpu_state.status_ & CPU_State::kZero) ? 'Z' : '-',
                    (cpu_state.status_ & CPU_State::kCarry) ? 'C' : '-');

            TextFmt("A:{:02x} X:{:02x} Y:{:02x} SP:{:02x}\n",
                    cpu_state.accumulator_, cpu_state.register_x_, cpu_state.register_y_, cpu_state.stack_pointer_);

            TextFmt("PC:{:04X}", cpu_state.program_counter_);

            NewLine();
            SeparatorText("Stepping");

            BeginDisabled(!emulator.is_ready());
            Debugger& debugger = emulator.debugger_;

            if (Button("Single"))
                debugger.request_break(Debugger::MODE_CPU_FETCH);

            SameLine();
            if (Button("Line"))
                debugger.request_break(Debugger::MODE_PPU_LINE);

            SameLine();
            if (Button("Frame"))
                debugger.request_break(Debugger::MODE_PPU_FRAME);

            BeginDisabled(!emulator.is_debugging());

            if (Button("Continue"))
                debugger.resume();

            EndDisabled();

            EndDisabled();
        }

        EndTable();
    }
}