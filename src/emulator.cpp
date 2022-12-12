#include "emulator.h"

#include "types.h"
#include "clock.h"

#include <fmt/core.h>

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <memory>

#include <iostream>

Emulator::Emulator()
    : cpu_(new CPU)
    , apu_(new APU)
    , ppu_(new PPU)
    , ram_(new RAM)
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

void Emulator::read_rom(const std::string& filename)
{
    cart_.reset();

    bifstream ifs(filename, std::ios_base::binary);

    if (ifs)
    {
        byte_t header[16];
        ifs.read(header, 16);

        fmt::print("Loading {}\n\n", filename);

        if (std::memcmp(header, "NES", 3) != 0 && header[3] != 0x1a)
            throw std::runtime_error("Bad file format");

        bool nes20 = (0x3 & (header[7] >> 2)) == 2;
        if (nes20)
            throw std::runtime_error("NES 2.0 header not yet supported.");

        
        Mirroring mir = Mirroring::None;
        if ((0x1 & (header[6] >> 3)) == 0)
        {
            mir = (0x1 & header[6]) ? Mirroring::Vertical : Mirroring::Horizontal;
        }

        ppu_->set_mirroring(mir);

        bool battery = 0x1 & (header[6] >> 1);
        bool trainer = 0x1 & (header[6] >> 2);
        bool fourscreen = 0x1 & (header[6] >> 3);

        byte_t mapper = (byte_t)((header[7] & 0xF0) | (header[6] >> 4));
        fmt::print("mapper: {:03}\n\n", mapper);
        cart_.reset(new Cartridge(mapper));

        fmt::print("config:\n    mirroring: {}\n    battery: {}\n    trainer: {}\n    four-screen: {}\n\n", 
            (mir == Mirroring::Vertical ? "vertical" : "horizontal"),
            battery,
            trainer,
            fourscreen,
            (0x1 & (header[7] >> 0)),
            (0x1 & (header[7] >> 1))
        );


        byte_t num_16kb_prg_rom_banks = header[4];
        fmt::print("num_16kb_prg_rom_banks: {}\n", num_16kb_prg_rom_banks);

        byte_t num_8kb_chr_rom_banks = header[5];
        fmt::print("num_8kb_chr_rom_banks: {}\n", num_8kb_chr_rom_banks);

        byte_t num_8kb_prg_ram_banks = header[8];
        if (num_8kb_prg_ram_banks == 0)
            num_8kb_prg_ram_banks = 1; // compatibility

        //if (0x1 & (header[10] >> 4))
        //    num_8kb_prg_ram_banks = 1; // ??
        fmt::print("num_8kb_prg_ram_banks: {}\n", num_8kb_prg_ram_banks);

        ifs.seekg(16);

        // trainer is 512B, skip it for now
        if (trainer)
            ifs.ignore(512);

        cart_->load_roms(ifs, num_16kb_prg_rom_banks, num_8kb_chr_rom_banks, num_8kb_prg_ram_banks);
        bus_->load_cartridge(cart_.get());

        disassembler_.load(cart_.get());
    }
    else
    {
        throw std::runtime_error(fmt::format("Can't open file {}", filename));
    }
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

    if (!is_paused() || steps_ > 0)
    {
        cpu_cycle_start_of_frame = get_cpu()->get_state().cycle_;

        // NTSC emulation: 29780.5 cpu cycles per frame: ~60 Hz
        while (!ppu_->grab_frame_done())
        {
            try
            {
                for (int i = 0; i < 3; ++i)
                    ppu_->step();

                // Debugging vblank timing
                uint64_t cycle = get_cpu()->get_state().cycle_;
                if (ppu_->get_state().is_in_vblank_ && cpu_cycle_at_vblank == 0)
                    cpu_cycle_at_vblank = cycle;
                else if (!ppu_->get_state().is_in_vblank_ && cpu_cycle_at_vblank != 0)
                {
                    cpu_cycle_last_vblank = cycle - cpu_cycle_at_vblank;
                    cpu_cycle_at_vblank = 0;
                }

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
            catch (std::exception e)
            {
                fmt::print("Exception: {}\n", e.what());
                NES_BREAKPOINT;
                mode_ = Mode::PAUSED;
                break;
            }
            catch (...)
            {
                NES_BREAKPOINT;
                mode_ = Mode::PAUSED;
                break;
            }

            cycle_++;

            if (mode_ == Mode::STEP_ONCE && cpu_->get_state().state_ == CPU_State::kFetching)
            {
                break;
            }

            if (mode_ == Mode::STEP_LINE && ppu_->is_at_end_of_line())
            {
                break;
            }
        }

        {
            uint64_t cpu_cycle = get_cpu()->get_state().cycle_;
            cpu_cycle_per_frame = cpu_cycle - cpu_cycle_start_of_frame;
        }

        if (steps_ > 0)
            --steps_;
    }
}

void Emulator::press_button(Controller::Button button)
{
    bus_->ctrl_.press(button);
}

void Emulator::release_button(Controller::Button button)
{
    bus_->ctrl_.release(button);
}