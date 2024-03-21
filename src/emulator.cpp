#include "emulator.h"

#include "types.h"
#include "clock.h"
#include "ines.h"

#include "ui/global.h"

#include <fmt/core.h>
#include <fmt/xchar.h>

#include <stdexcept>

Emulator::Emulator()
    : cpu_(new CPU)
    , apu_(new APU)
    , ppu_(new PPU)
    , ram_(new RAM)
    , debugger_(*this)
{
    bus_.reset(new BUS(*cpu_, *apu_, *ppu_, *ram_));
    cpu_->init(bus_.get());
    ppu_->init(bus_.get());

    NES_ASSERT(instance_ == nullptr);
    instance_ = this;
}

Emulator::~Emulator()
{
    memset(&instance_, 0xDEAD, sizeof(instance_));
}

void Emulator::read_rom(std::wstring_view filename)
{
    cart_.reset();
    
    fmt::print(L"Loading {}\n\n", filename);

    INESReader reader;
    if (!reader.read_from_file(filename))
        throw std::runtime_error("Can't open file");

    INESHeader& h = reader.header_;

    auto mirroring_name = [mir = h.mirroring_]
    {
        using namespace std::literals;

        switch (mir)
        {
        case NT_Mirroring::None:
            return "four-screen"sv;

        case NT_Mirroring::Single:
            return "single-screen"sv;

        case NT_Mirroring::Horizontal:
            return "horizontal"sv;

        case NT_Mirroring::Vertical:
            return "vertical"sv;
        }

        return "UNKNOWN"sv;
    };

    fmt::print("config:\n    iNES version: {}\n    mirroring: {}\n    battery: {}\n    trainer: {}\n\n",
        h.is_ines2_ ? "2" : "1",
        mirroring_name(),
        h.has_prg_ram_,
        h.has_trainer_);

    // Compatibility
    if (!h.is_ines2_ && h.prg_ram_size_ == 0)
        h.prg_ram_size_ = 1;

    fmt::print("num_16kb_prg_rom_banks: {}\n", h.prg_rom_size_);
    fmt::print("num_8kb_chr_rom_banks: {}\n", h.chr_rom_size_);
    fmt::print("num_8kb_prg_ram_banks: {}\n", h.prg_ram_size_);

    cart_.reset(new Cartridge(h.mapper_));
    cart_->load_roms(reader);

    ppu_->set_mirroring(h.mirroring_);

    bus_->load_cartridge(cart_.get());
    disassembler_.load(cart_.get());

    ui::push_recent_file(filename);

    reset();
}

void Emulator::reset()
{
    cpu_->reset();
    apu_->reset();
    ppu_->reset();
    cycle_ = 0;
    dma_cycle_counter_ = 0;
}

void Emulator::update()
{
    if (!is_ready())
        return;

    if (is_stepping())
    {
        cpu_cycle_start_of_frame = get_cpu()->get_state().cycle_;
        ppu_cycle_start_of_frame = get_ppu()->get_state().cycle_counter_;

        while (!ppu_->grab_frame_done())
        {
            try
            {
                if (cycle_ % 3 == 0)
                    clock_cpu_();

                clock_ppu_();
            }
            catch (std::exception e)
            {
                fmt::print("Exception: {}\n", e.what());
                NES_BREAKPOINT;
                debugger_.break_now();
                break;
            }
            catch (...)
            {
                NES_BREAKPOINT;
                debugger_.break_now();
                break;
            }

            cycle_++;
            
            if (debugger_.get_mode() != Debugger::MODE_RUNNING)
                break;
        }

        {
            const uint64_t cpu_cycle = get_cpu()->get_state().cycle_;
            cpu_cycle_per_frame = cpu_cycle - cpu_cycle_start_of_frame;

            const uint64_t ppu_cycle = get_ppu()->get_state().cycle_counter_;
            ppu_cycle_per_frame = ppu_cycle - ppu_cycle_start_of_frame;
        }
    }
}

void Emulator::toggle_pause()
{
    if (is_debugging())
    {
        paused_ = false;
        debugger_.resume();
        return;
    }

    paused_ = !paused_;
}

void Emulator::press_button(Controller::Button button)
{
    bus_->ctrl_.press(button);
}

void Emulator::release_button(Controller::Button button)
{
    bus_->ctrl_.release(button);
}

// NTSC emulation: 29780.5 cpu cycles per frame: ~60 Hz
void Emulator::clock_cpu_()
{
    apu_->step();

    if (dma_cycle_counter_ == 0)
    {
        cpu_->step();
    }
    else
    {
        --dma_cycle_counter_;
        if ((dma_cycle_counter_ & 0x1) == 0 && dma_cycle_counter_ <= 512)
            ppu_->dma_copy_byte(256_byte - static_cast<byte_t>(dma_cycle_counter_ / 2));
        cpu_->dma_clock();
    }

    if (ppu_->grab_dma_request())
        dma_cycle_counter_ = 513 + (cpu_->get_state().cycle_ & 0x1);
}

void Emulator::clock_ppu_()
{
    ppu_->step();

    // Debugging vblank timing
    uint64_t cycle = get_cpu()->get_state().cycle_;
    if (ppu_->get_state().is_in_vblank_ && cpu_cycle_at_vblank == 0)
    {
        cpu_cycle_at_vblank = cycle;
    }
    else if (!ppu_->get_state().is_in_vblank_ && cpu_cycle_at_vblank != 0)
    {
        cpu_cycle_last_vblank = cycle - cpu_cycle_at_vblank;
        cpu_cycle_at_vblank = 0;
    }
}