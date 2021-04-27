#include "emulator.h"

#include "types.h"
#include "sfml_renderer.h"

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
}

void Emulator::read_rom(const std::string& filename)
{
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

        disassembler_.load(cart_.get());

        bus_.reset(new BUS(*cpu_, *apu_, *ppu_, *ram_, *cart_));
        cpu_->init(bus_.get());
        ppu_->init(bus_.get(), cart_.get());
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
    dma_cycle_counter = 0;
}

int Emulator::run()
{
    SFMLRenderer renderer(this);

    cpu_->reset();
    apu_->reset();
    ppu_->reset();

    for (;;)
    {
        if (renderer.timeout())
        {
            if (mode_ != Mode::PAUSED)
            {
                // NTSC emulation: 29780.5 cpu cycles per frame: ~60 Hz
                while (!ppu_->grab_frame_done())
                {
                    try
                    {
                        if (ppu_->grab_dma_request())
                            dma_cycle_counter = 513 + (cpu_->get_state().cycle_ & 0x1);

                        ppu_->step();

                        {
                        if ((cycle_ % 3) == 0)
                            apu_->step();

                            if (dma_cycle_counter == 0)
                            {
                                cpu_->step();
                            }
                            else
                            {
                                if ((dma_cycle_counter & 0x1) == 0 && dma_cycle_counter <= 512)
                                    ppu_->dma_copy_byte(256 - (dma_cycle_counter / 2));
                                --dma_cycle_counter;
                            }
                        }
                    }
                    catch (std::exception e)
                    {
                        BREAKPOINT;
                        mode_ = Mode::PAUSED;
                        break;
                    }
                    catch (...)
                    {
                        BREAKPOINT;
                        mode_ = Mode::PAUSED;
                        break;
                    }

                    if (mode_ == Mode::STEP_ONCE && cpu_->get_state().idle_ticks_ == 0)
                    {
                        ++cycle_;
                        break;
                    }
                }

                if (mode_ != Mode::RUN)
                    mode_ = Mode::PAUSED;
            }

            if (!renderer.update())
                break;
        }
    }

    return 0;
}