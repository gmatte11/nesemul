#include "ui/imgui_debugger.h"
#include "ui/imgui.h"

#include "emulator.h"
#include "cartridge.h"
#include "cpu.h"
#include "ops.h"

#include "types.h"
#include "utils.h"

#include <ranges>

struct PrgModel
{
    using PrgBank = Disassembler::PrgBank;
    using Op = Disassembler::Op;

    struct OpRange
    {
        const PrgBank* decoded_bank_;
        address_t mapped_addr = 0_addr;
        int first_idx_;
        int size_;
    };

    std::array<OpRange, 8> ops_;

    void clear()
    {
        ops_.fill({ nullptr, 0, 0 });
    }

    void reset(const BUS& bus, int prg_idx)
    {
        clear();

        if (bus.cart_ == nullptr)
            return;

        Disassembler& disassembler = Emulator::instance()->disassembler_;

        if (prg_idx == 0)
        {
            const MemoryMap& prg_map = bus.cart_->get_mapped_prg();

            auto it = ops_.begin();
            for (const auto& memory_bank : prg_map.map_)
            {
                for (const PrgBank& decoded_bank : disassembler.get_banks())
                {
                    const byte_t* mem = memory_bank.data_.data();
                    if (mem >= decoded_bank.rom_ && mem < decoded_bank.rom_ + decoded_bank.rom_size_)
                    {
                        //NES_ASSERT(mem + memory_bank.data_.size() <= decoded_bank.rom_ + decoded_bank.rom_size_);
                        *it = decode_op_range(decoded_bank, memory_bank);
                    }
                }

                ++it;
            }
        }
        else
        {
            // TODO?
            /*
            const Disassembler::PrgBank* bank1 = nullptr;
            const Disassembler::PrgBank* bank2 = nullptr;

            const int disassembler_bank_idx = (prg_idx - 1) * 2; 

            auto banks = disassembler.get_banks();
            bank1 = &banks[disassembler_bank_idx];
            bank2 = &banks[disassembler_bank_idx + 1];

            banks_[0] = bank1;
            banks_[1] = bank2;

            if (banks_[0] != nullptr)
                sizes_[0] = static_cast<int>(banks_[0]->ops_.size());

            if (banks_[1] != nullptr)
                sizes_[1] = static_cast<int>(banks_[1]->ops_.size());
            */
        }
    }

    static OpRange decode_op_range(const PrgBank& decoded_bank, const MemoryMap::Bank& memory_bank)
    {
        const byte_t* mem = memory_bank.data_.data();
        const size_t bank_sz = memory_bank.data_.size();

        auto it = decoded_bank.ops_.begin();

        int first_idx = 0;

        const byte_t* op_ptr = decoded_bank.rom_;
        while (op_ptr < mem)
        {
            const int width = ops::opcode_data(it->opcode_).get_size();
            op_ptr += width;
            ++first_idx;
        }

        int size = 0;

        while (op_ptr < mem + bank_sz)
        {
            const int width = ops::opcode_data(it->opcode_).get_size();
            op_ptr += width;
            ++size;
        }

        return { &decoded_bank, static_cast<address_t>(memory_bank.addr_ + first_idx), first_idx, size };
    }

    int size() const
    {
        return std::ranges::fold_left(ops_, 0, [](int init, const OpRange& rhs) { return init + rhs.size_; });
    }

    Op get_op(int idx)
    {
        int idx_bank = 0;

        while (idx > ops_[idx_bank].size_)
        {
            idx -= ops_[idx_bank].size_;
            idx_bank++;
        }

        const OpRange& range = ops_[idx_bank];
        const PrgBank* decoded_bank = range.decoded_bank_;

        Op op = decoded_bank->ops_[range.first_idx_ + idx];
        op.addr_ += range.mapped_addr + static_cast<address_t>(idx);

        return op;
    }

    int get_idx(address_t addr)
    {
        for (int i = 0; i < ops_.size(); ++i)
        {
            const OpRange& op = ops_[i];
            if (addr >= op.mapped_addr && addr < op.mapped_addr + op.size_)
                return i;
        }

        return -1;
    }



    void* cart_check = nullptr;
};

std::string cpu_flags_str(byte_t state_flags)
{
    return fmt::format("{}{}xx{}{}{}{}",
                       (state_flags & CPU_State::kNegative) ? 'N' : '-',
                       (state_flags & CPU_State::kOverflow) ? 'O' : '-',
                       (state_flags & CPU_State::kDecimal) ? 'D' : '-',
                       (state_flags & CPU_State::kIntDisable) ? 'I' : '-',
                       (state_flags & CPU_State::kZero) ? 'Z' : '-',
                       (state_flags & CPU_State::kCarry) ? 'C' : '-');
}

void breakpoints(Debugger& debugger)
{
    using namespace imgui;
    const float textHeight = GetTextLineHeightWithSpacing();
    const float textWidth = CalcTextSize("A").x;

    using Reason = Breakpoint::Reason;
    static Breakpoint input_cb;
    bool do_add = false;

    auto remove_it = debugger.breakpoints_.end();

    static constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;

    if (BeginTable("Breakpoints", 3, table_flags, ImVec2{0.f, textHeight * 4}))
    {
        const ImVec2 buttonPadding = GetStyle().FramePadding;
        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 2.f));

        TableSetupColumn("Reason", 0, 0.f, 0);
        TableSetupColumn("Value", 0, 0.f, ImGuiTableColumnFlags_Disabled);
        TableSetupColumn("##Action", 0, 0.f, ImGuiTableColumnFlags_Disabled);

        TableNextRow();
        bool first_row = true;

        {
            TableSetColumnIndex(0);
            PushItemWidth(textWidth * 10.f);
            TableSetColumnIndex(1);
            PushItemWidth(textWidth * 10.f);
            TableSetColumnIndex(2);
            PushItemWidth(textWidth * 3.f);
        }

        for (auto it = debugger.breakpoints_.begin(), e = debugger.breakpoints_.end(); it != e; ++it)
        {
            PushID(&*it);

            if (!first_row)
                TableNextRow();
            first_row = false;

            Breakpoint cb = *it;

            TableSetColumnIndex(0);
            AlignTextToFramePadding();
            TextUnformatted(cb.reason_.to_string().data());

            TableNextColumn();
            AlignTextToFramePadding();
            switch (cb.reason_)
            {
            case Reason::Addr:
                TextFmt("{:04X}", std::get<address_t>(cb.break_value_));
                break;

            case Reason::Opcode:
            {
                byte_t opcode = std::get<byte_t>(cb.break_value_);
                ops::metadata const& meta = ops::opcode_data(opcode);
                TextFmt("{:02X} {}", opcode, meta.str);
                break;
            }

            case Reason::Flag:
            {
                std::string str = cpu_flags_str(std::get<byte_t>(cb.break_value_));
                TextUnformatted(str.data(), str.data() + str.length());
                break;
            }

            }

            TableNextColumn();
            PushStyleVar(ImGuiStyleVar_FramePadding, buttonPadding);
            if (Button("X"))
                remove_it = it;
            PopStyleVar();

            PopID();
        }

        if (!first_row)
            TableNextRow();

        TableSetColumnIndex(0);
        if (BeginCombo("##reason", input_cb.reason_.to_string().data()))
        {
            for (int i = 0; i < Reason::get_strings().size() - 1; ++i)
            {
                const bool selected = i == input_cb.reason_;
                if (Selectable(Reason::get_strings()[i].data(), selected))
                {
                    input_cb.reason_ = Reason::from_value(i);

                    if (i == 0)
                        input_cb.break_value_ = 0_addr;
                    else
                        input_cb.break_value_ = 0_byte;
                }

                if (selected)
                    SetItemDefaultFocus();
            }
            EndCombo();
        }

        TableNextColumn();
        const bool is_addr = input_cb.reason_ == Reason::Addr;

        const ImGuiDataType data_type = is_addr ? ImGuiDataType_U16 : ImGuiDataType_U8;
        void* p_data = is_addr ? (void*)&std::get<address_t>(input_cb.break_value_) : (void*)&std::get<byte_t>(input_cb.break_value_);

        InputScalar("##value", data_type, p_data, nullptr, nullptr, is_addr ? "%04X" : "%02X");

        TableNextColumn();
        PushStyleVar(ImGuiStyleVar_FramePadding, buttonPadding);
        if (Button("+"))
            do_add = true;
        PopStyleVar();

        EndTable();
        PopStyleVar();
    }

    if (remove_it != debugger.breakpoints_.end())
        debugger.breakpoints_.erase(remove_it);

    if (do_add)
    {
        debugger.breakpoints_.push_back(input_cb);
        input_cb = {};
    }
}

void ui::imgui_debugger()
{
    using namespace imgui;
    Emulator& emulator = *Emulator::instance();
    BUS& bus = *emulator.get_bus();
    CPU& cpu = *emulator.get_cpu();
    CPU_State const& cpu_state = cpu.get_state();

    static PrgModel model;
    model.reset(bus, 0);

    address_t program_counter = bus.map_cpu_addr(cpu_state.program_counter_);

    int sticky_idx = model.get_idx(program_counter);

    const float textHeight = GetTextLineHeightWithSpacing();

    if (BeginTable("layout_columns", 2))
    {
        TableNextRow();
        TableNextColumn();

        {
            SeparatorText("Instructions");
            
            static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
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
                    clipper.IncludeItemByIndex(sticky_idx);

                while (clipper.Step())
                {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
                    {
                        PrgModel::Op op = model.get_op(row);
                        const ops::metadata& meta = ops::opcode_data(op.opcode_);

                        const bool is_at_pc = sticky_idx >= 0 && sticky_idx == row;

                        TableNextRow();

                        TableNextColumn();
                        const address_t mapped_addr = bus.map_cpu_addr(op.addr_);
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

            TextFmt("Flags: {} [{:02X}]\n", cpu_flags_str(cpu_state.status_), cpu_state.status_);

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

            NewLine();
            SeparatorText("Breakpoints");
            breakpoints(emulator.debugger_);

            EndDisabled();
        }

        EndTable();
    }
}