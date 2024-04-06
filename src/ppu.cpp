#include "ppu.h"

#include "debugger.h"
#include "ram.h"


void PPU::step()
{
    bg_eval_();
    fg_eval_();
    render_();
    tick_();
}

void PPU::reset()
{
    scanline_ = -1;
    cycle_ = 0;
    frame_ = 0;
    cycle_counter_ = 0;

    ppuctrl_.set(0);
    ppumask_.set(0);

    scroll_x_ = 0;
    scroll_y_ = 0;
    is_in_vblank_ = false;

    cursor_.v.set(0);
    cursor_.t.set(0);
    cursor_.x = 0;
    cursor_.w = false;
    read_buffer_ = 0;

    dma_requested_ = false;

    oam_.fill(0xFF);
}

bool PPU::on_write_cpu(address_t addr, byte_t value)
{
    // Mirroring
    if (addr >= 0x2008 && addr < 0x4000)
        addr = 0x2000 + (addr & 0x7);

    switch (addr)
    {
        // ppuctrl
    case 0x2000:
        {
        const bool was_nmi_enabled = ppuctrl_.nmi_;
        ppuctrl_.set(value);

        if (ppustatus_.vblank_ && !was_nmi_enabled && ppuctrl_.nmi_)
            bus_->cpu_.pull_nmi();

        cursor_.t.NX = ppuctrl_.nam_x_;
        cursor_.t.NY = ppuctrl_.nam_y_;
        }
        return true;

        // ppumask
    case 0x2001:
        ppumask_.set(value);
        return true;

        // oamaddr
    case 0x2003:
        oamaddr_ = value;
        return true;

        //oamdata
    case 0x2004:
        oam_[oamaddr_] = value;
        ++oamaddr_;
        return true;

        // ppuscroll
    case 0x2005:
    {
        if (!cursor_.w)
        {
            scroll_x_ = value;
            cursor_.t.X = (value >> 3) & 0x1F;
            cursor_.x = value & 0x07;
        }
        else
        {
            scroll_y_ = value;
            cursor_.t.Y = (value >> 3) & 0x1F;
            cursor_.t.y = value & 0x07;
        }

        cursor_.w = !cursor_.w;
        return true;
    }

        // ppuaddr
    case 0x2006:
    {
        if (!cursor_.w)
        {
            cursor_.t.set_h(value & 0x3F);
        }
        else
        {
            cursor_.t.set_l(value);
            cursor_.v = cursor_.t;
        }

        cursor_.w = !cursor_.w;
        return true;
    }

        // ppudata
    case 0x2007:
        store_(cursor_.v.get() & 0x3FFF, value);
        cursor_.v += (ppuctrl_.addr_inc_) ? 32 : 1;
        cursor_.v &= 0x7FFF;
        return true;

        // oamdma
    case 0x4014:
    {
        //NES_ASSERT(oamaddr_ == 0);

        dma_requested_ = true;
        dma_page_idx_ = value;

        return true;
    }
    }

    return false;
}

bool PPU::on_read_cpu(address_t addr, byte_t& value)
{
    // Mirroring
    if (addr >= 0x2008 && addr < 0x4000)
        addr = 0x2000 + (addr & 0x7);

    switch (addr)
    {
    case 0x2002:
        value = ppustatus_.get();
        ppustatus_.vblank_ = 0;
        cursor_.w = false;

        if (scanline_ == 241 && cycle_ <=  1)
            suppress_vblank_ = true;

        return true;

    case 0x2004:
        value = oam_[oamaddr_];
        return true;

    case 0x2007:
    {
        const address_t vram_addr = cursor_.v.get() & 0x3FFF;

        value = read_buffer_;
        read_buffer_ = load_(vram_addr);

        // palette read is instantaneous
        if (vram_addr >= 0x3F00 && vram_addr < 0x4000)
            value = read_buffer_;

        cursor_.v += (ppuctrl_.addr_inc_) ? 32 : 1;
        cursor_.v &= 0x7FFF;

        return true;
    }
    }

    return false;
}

bool PPU::on_write_ppu(address_t addr, byte_t value)
{
    addr = mirror_addr_(addr);
    
    if (addr >= 0x2000 && addr < 0x3000)
    {
        memory_[addr - 0x2000] = value;
        return true;
    }

    if (addr >= 0x3F00 && addr < 0x3F20)
    {
        palette_[addr & 0xFF] = value;
        return true;
    }

    return false;
}

bool PPU::on_read_ppu(address_t addr, byte_t& value)
{
    addr = mirror_addr_(addr);
    
    if (addr >= 0x2000 && addr < 0x3000)
    {
        value = memory_[addr - 0x2000];
        return true;
    }

    if (addr >= 0x3F00 && addr < 0x3F20)
    {
        value = palette_[addr & 0xFF];
        return true;
    }

    return false;
}

void PPU::dma_copy_byte(byte_t rw_cycle)
{
    NES_ASSERT(rw_cycle >= 0 && rw_cycle < 256);
    address_t page_addr = dma_page_idx_ << 8;

    byte_t data = bus_->read_cpu(page_addr | rw_cycle);
    oam_[rw_cycle] = data;
}

void PPU::bg_eval_()
{
    auto& v = cursor_.v;

    if (!ppumask_.render_bg_ && !ppumask_.render_fg_)
        return;

    if (scanline_ >= -1 && scanline_ < 240)
    {
        if (cycle_ == 256)
        {
            // Vertical increment
            if (v.y < 7)
            {
                v.y++;
            }
            else
            {
                v.y = 0;
                if (v.Y == 29)
                {
                    v.Y = 0;
                    v.NY = ~v.NY;
                }
                else if (v.Y == 31)
                {
                    v.Y = 0;
                }
                else
                {
                    v.Y++;
                }
            }
        }

        // Transfer horizontal scroll bits
        if (cycle_ == 257)
        {
            v.X = cursor_.t.X;
            v.NX = cursor_.t.NX;
        }

        // up to cycle 337 to shift the bg_shifter_  once more.
        if ((cycle_ > 0 && cycle_ <= 256) || (cycle_ > 320 && cycle_ <= 337))
        {
            Tile& tile = bg_next_tile_;

            bg_shifter_.shift();

            int op = (cycle_ - 1) % 8;
            switch (op)
            {
            case 0:
                bg_shifter_.load(tile);
                break;

            case 1: // NT byte
            {
                address_t ntaddr = 0x2000 | (v.get() & 0x0FFF);
                tile.ntbyte_ = load_(ntaddr);
            }
            break;

            case 3: // AT byte
            {
                address_t ataddr = 0x23C0 | (v.get() & 0x0C00) | ((v.get() >> 4) & 0x38) | ((v.get() >> 2) & 0x07);
                byte_t areashift = ((v.Y & 0x02) ? 4 : 0) + ((v.X & 0x02) ? 2 : 0);
                tile.atbyte_ = (load_(ataddr) >> areashift) & 0x3;
            }
            break;

            case 5: // Low PAT byte
            {
                tile.half_ = ppuctrl_.bg_pat_ & 0x1;

                address_t addr
                    = static_cast<address_t>(tile.half_) << 12
                    | static_cast<address_t>(tile.ntbyte_) << 4
                    | static_cast<address_t>(v.y);

                tile.lpat_ = load_(addr);
            }
            break;

            case 7: // High PAT byte
            {
                address_t addr
                    = static_cast<address_t>(tile.half_) << 12
                    | static_cast<address_t>(tile.ntbyte_) << 4
                    | static_cast<address_t>(v.y);

                tile.hpat_ = load_(addr + 8);

                // Increment horizontal scroll bits
                {
                    if (v.X == 31)
                    {
                        v.X = 0;
                        v.NX = ~v.NX;
                    }
                    else
                    {
                        v.X++;
                    }
                }
                break;
            }
            }
        }
    }
}

void PPU::fg_eval_()
{
    if (!ppumask_.render_fg_ && !ppumask_.render_bg_)
        return;

    if (scanline_ > 0 && scanline_ < 240)
    {
        if (cycle_ == 1)
        {
            secondary_oam_.flip();
            secondary_oam_.store().reset();
        }

        if (cycle_ == 256)
        {
            for (int i = 0; i < 64; ++i)
            {
                const OAMSprite& oam_sprite = reinterpret_cast<OAMSprite*>(oam_.data())[i];

                const bool long_sprite = ppuctrl_.sprite_size_;
                const int height = long_sprite ? 16 : 8;
                const int sprite_y = scanline_ - oam_sprite.y_;
                const bool is_visible = sprite_y >= 0 && sprite_y < height;
                const bool bottom_half = long_sprite && sprite_y >= 8;

                if (is_visible)
                {
                    auto& sprites = secondary_oam_.store();
                    if (sprites.count_ < 8)
                    {
                        SecondaryOAM::Entry& sprite = sprites.list_[sprites.count_];

                        if (i == 0) 
                            sprites.has_sprite_0_ = true;
                        
                        byte_t pat = long_sprite ? oam_sprite.tile_ & 0x1 :  ppuctrl_.fg_pat_;
                        byte_t ntidx = long_sprite ? oam_sprite.tile_ & 0xFE : oam_sprite.tile_;

                        if (bottom_half)
                            ntidx++;

                        Tile tile;
                        tile.ntbyte_ = ntidx;
                        tile.half_ = pat;

                        load_sprite_(sprite, tile, oam_sprite.att_, oam_sprite.x_, static_cast<byte_t>(sprite_y) & 0b111);

                        ++sprites.count_;
                    }
                    else
                    {
                        // TODO? Sprite overflow bug
                        ppustatus_.sprite_overflow_ = 1;
                    }
                }
            }
        }

        if (cycle_ > 256 && cycle_ <= 320)
        {
            oamaddr_ = 0;
        }
    }
}

void PPU::render_()
{
    if (scanline_ >= 0 && scanline_ < 240)
    {
        if (cycle_ > 0 && cycle_ <= 256)
        {
            int row = scanline_;
            int col = cycle_ - 1; // rendering of x=0 starts at cycle 1, x=255 at cycle 256.
            
            byte_t bg_pixel = 0;
            byte_t fg_pixel = 0;
            byte_t bg_pal = 0;
            byte_t fg_pal = 0;
            bool fg_priority = false;
            bool is_sprite_0 = false;

            if (ppumask_.render_bg_ && (col > 7 || ppumask_.left_bg_))
            {
                address_t mux = 0x8000 >> cursor_.x;

                bg_pixel = ((bg_shifter_.hpat_ & mux) ? 0b10 : 0b00) | ((bg_shifter_.lpat_ & mux) ? 0b01 : 0b00);
                bg_pal = ((bg_shifter_.hatt_ & mux) ? 0b10 : 0b00) | ((bg_shifter_.latt_ & mux) ? 0b01 : 0b00);
            }

            if (ppumask_.render_fg_ && (col > 7 || ppumask_.left_fg_))
            {
                auto& sprites = secondary_oam_.read();

                for (int i = 0; i < sprites.count_ && fg_pixel == 0; ++i)
                {
                    SecondaryOAM::Entry& sprite = sprites.list_[i];
                    byte_t sprite_pat = sprite.get_pat(static_cast<byte_t>(col));

                    if (sprite_pat != 0)
                    {
                        fg_pixel = sprite_pat;
                        fg_pal = (sprite.att_ & 0x3) + 4;
                        fg_priority = (sprite.att_ & 0x20) == 0;
                        
                        is_sprite_0 = i == 0 && sprites.has_sprite_0_;
                    }
                }
            }

            const bool sprite_0_eval = is_sprite_0 && ppumask_.render_fg_ && ppumask_.render_bg_ && col < 255;

            if (sprite_0_eval)
            {
                if (fg_pixel != 0 && bg_pixel != 0)
                    ppustatus_.sprite_0_hit_ = 1;
            }

            const bool is_fg_pixel = ppumask_.render_fg_ && fg_pixel != 0 && (bg_pixel == 0 || fg_priority);
            const bool is_bg_pixel = ppumask_.render_bg_; // only if the foreground pixel doesn't have priority.
            
            Color color{0, 0, 0, 0xFF};

            if (is_fg_pixel)
            {
                color = ppu_read_palette(*bus_, fg_pal).get(fg_pixel);
            }
            else if (is_bg_pixel)
            {
                color = ppu_read_palette(*bus_, bg_pal).get(bg_pixel);
            }

            // TODO: emphasis color

            output_.set(col, row, color);
        }
    }
}

void PPU::tick_()
{
    // scanline -1: dummy scanline, single cycle skipped on odd-frame, vblank is unset.
    // scanlines 0..239: rendering scanlines
    // scanline  240: empty scanline.
    // scanlines 241-260: vblank is set a the 2nd cycle of the 241st scanline and also trigger the vblank's NMI.

    const bool rendering_enabled = ppumask_.render_bg_ || ppumask_.render_fg_;
    
    if (scanline_ == -1)
    {
        // cycle skip on odd frame
        if (rendering_enabled && cycle_ == 339 && (frame_ & 0x1) != 0)
            ++cycle_;
    }

    ++cycle_counter_;

    ++cycle_;
    if (cycle_ > 340)
    {
        cycle_ = 0;
        ++scanline_;

        Debugger::instance()->on_ppu_line();

        if (scanline_ > 260)
        {
            scanline_ = -1;
            ++frame_;
            frame_done_ = true;
            Debugger::instance()->on_ppu_frame();
        }
    }

    // dummy (pre-render) scanline
    if (scanline_ == -1)
    {
        if (cycle_ == 1)
        {
            ppustatus_.vblank_ = 0; // end of vblank
            is_in_vblank_ = false;
            ppustatus_.sprite_0_hit_ = 0;
            ppustatus_.sprite_overflow_ = 0;
        }

        // Transfer vertical scroll bits
        if (rendering_enabled && cycle_ >= 280 && cycle_ <= 304)
        {
            cursor_.v.Y = cursor_.t.Y;
            cursor_.v.y = cursor_.t.y;
            cursor_.v.NY = cursor_.t.NY;
        }
    }

    // first scanline of vblank
    if (scanline_ == 241 && cycle_ == 1)
    {
        if (!suppress_vblank_)
            ppustatus_.vblank_ = 1; // start of vblank
        suppress_vblank_ = false;
        is_in_vblank_ = true;
    }

    if (scanline_ == 241 && cycle_ == 20)
    {
        if (ppustatus_.vblank_ && ppuctrl_.nmi_)
            bus_->cpu_.pull_nmi();
    }
}

void PPU::SecondaryOAM::reset()
{
    static constexpr SecondaryOAM::Entry empty{0xFF, 0xFF, 0xFF, 0xFF};
    
    count_ = 0;
    has_sprite_0_ = false;
    list_.fill(empty);
}

void PPU::load_sprite_(SecondaryOAM::Entry& sprite, Tile tile, byte_t attrib, byte_t x, byte_t ty)
{
    sprite.hpat_ = sprite.lpat_ = 0;
    sprite.att_ = attrib;
    sprite.x_ = x;

    byte_t flip = attrib >> 6;
    if (0b10 & flip) ty = 7 - ty; //vert flip

    address_t addr = static_cast<address_t>(tile.half_ & 0x1) << 12
        | static_cast<address_t>(tile.ntbyte_) << 4
        | static_cast<address_t>(ty & 0b111) << 0;
 
    byte_t lpat = load_(addr);
    byte_t hpat = load_(addr + 8);

    for (byte_t tx = 0; tx < 8; ++tx)
    {
        byte_t local_x = tx;
        if (0b01 & flip) local_x = 7 - tx; //hori flip
        sprite.lpat_ |= ((lpat >> local_x) & 0x1) << tx;
        sprite.hpat_ |= ((hpat >> local_x) & 0x1) << tx;
    }
}

byte_t PPU::load_(address_t addr) const
{
    return bus_->read_ppu(addr);
}

void PPU::store_(address_t addr, byte_t value)
{
    bus_->write_ppu(addr, value);
}

address_t PPU::mirror_addr_(address_t addr) const
{
    NES_ASSERT(addr < 0x4000);

    // $3F20-$3FFF mirrors $3F00-$3F1F
    if (addr >= 0x3F20 && addr < 0x4000)
        addr &= 0x3F1F;

    // Palette mirrors
    if (addr == 0x3F10 || addr == 0x3F14 || addr == 0x3F18 || addr == 0x3F1C)
        addr -= 0x10;

    // $3000-$3EFF mirrors $2000-$2EFF
    if (addr >= 0x3000 && addr < 0x3F00)
        addr -= 0x1000;

    switch (mirroring_)
    {
    case NT_Mirroring::Single:
        if (addr >= 0x2400 && addr < 0x3000)
            addr &= 0x23FF;
        break;

    case NT_Mirroring::Vertical:
        if (addr >= 0x2800 && addr < 0x3000)
            addr -= 0x0800;
        break;

    case NT_Mirroring::Horizontal:
        if (addr >= 0x2400 && addr < 0x2800)
            addr -= 0x0400;
        
        if (addr >= 0x2C00 && addr < 0x3000)
            addr -= 0x0400;
        break;

    default:
        break;
    }

    return addr;
}