#include "../bus.h"

static LoadColorsResult load_colors(PPU* ppu, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return LOAD_COLORS_OK_FILE_ERROR;
    }
    if (fread(ppu->colors, 1, 0x600, f) != 0x600) {
        fclose(f);
        return LOAD_COLORS_OK_READ_ERROR;
    }
    fclose(f);
    return LOAD_COLORS_OK;
}

int ppu_init(PPU* ppu, Bus* bus, MirroringMode mode) {
    memset(ppu, 0, sizeof(PPU));
    LoadColorsResult result = load_colors(ppu, "2C02G_wiki.pal");
    ppu->bus = bus;
    ppu->mirroring = mode;
}

void ppu_reset(PPU* ppu) {
    memset(&ppu->vram,0,0x2000);
    memset(&ppu->oam,0xFF,sizeof(OAMRAM));
    memset(&ppu->secondary_oam,0,sizeof(Sprite)*8);
    memset(&ppu->palette,0,0x20);
    memset(&ppu->pixel_buffer,0,0x100);
    memset(&ppu->framebuffer,0,0xF000);
    ppu->ppuctrl.value = 0;
    ppu->ppumask.value = 0;
    ppu->ppustatus.value = 0;
    ppu->oamaddr = 0;
    ppu->ppuaddr = 0;
    ppu->ppudata = 0;
    ppu->v.value = 0;
    ppu->t.value = 0;
    ppu->x = 0;
    ppu->w = 0;
    ppu->cycle = 0;
    ppu->scanline = 0;
    ppu->frame_odd = 0;
    ppu->open_bus_val = 0;
    ppu->frame_count = 0;
    ppu->sprite_count = 0;
}

uint8_t ppu_register_read(PPU* ppu, uint16_t address) {
    uint8_t result = 0;
    int increment = 0;
    switch(address) {
        case 2:
            result = ppu->ppustatus.value;
            ppu->ppustatus.VBLANK = 0;
            ppu->ppustatus.SP0_HIT = 0;
            ppu->w = 0;
            break;
        case 4:
            result = ppu->oam.raw[ppu->oamaddr];
            break;
        case 7:
            result = ppu->open_bus_val;
            ppu->open_bus_val = ppu_read(ppu, ppu->v.value);
            if (ppu->v.value >= 0x3F00) {
                result = ppu->open_bus_val;
            }
            increment = ppu->ppuctrl.VRAM_DIRECTION == 1 ? 32 : 1;
            ppu->v.value = (ppu->v.value + increment) & 0x3FFF;
            break;
        default:
            break;
    }
    return result;
}

void ppu_register_write(PPU* ppu, uint16_t address, uint8_t value) {
    int increment = 0;
    switch(address) {
        case 0:
            ppu->ppuctrl.value = value;
            ppu->t.nametable = value & 0x03;
            break;
        case 1:
            ppu->ppumask.value = value;
            break;
        case 3:
            ppu->oamaddr = value;
            break;
        case 4:
            ppu->oam.raw[ppu->oamaddr] = value;
            ppu->oamaddr = (ppu->oamaddr + 1) & 0xFF;
            break;
        case 5:
            if (ppu->w == 0) {
                ppu->x = value & 0x07;
                ppu->t.coarse_x = (value >> 3) & 0x1F;
            } else {
                ppu->t.fine_y = value & 0x07;
                ppu->t.coarse_y = (value >> 3) & 0x1F;
                ppu->t.nametable = (ppu->t.nametable & 0x01) | ((value >> 5) & 0x02);
            }
            ppu->w ^= 1;
            break;
        case 6:
            if (ppu->w == 0) {
                ppu->t.value = (ppu->t.value & 0x00FF) | ((value & 0x3F) << 8);
            } else {
                ppu->t.value = (ppu->t.value & 0xFF00) | value;
                ppu->v.value = ppu->t.value;
            }
            ppu->w ^= 1;
            break;
        case 7:
            ppu_write(ppu, ppu->v.value, value);
            increment = ppu->ppuctrl.VRAM_DIRECTION == 1 ? 32 : 1;
            ppu->v.value =(ppu->v.value + increment) & 0x3FFF;
        default:
            break;
    }
}

uint8_t ppu_read(PPU* ppu, uint16_t address) {
    if (address < 0x2000) {
        return ppu->bus->cart.mapper->read_chr(&ppu->bus->cart, address & 0x1FFF);
    } else if (address < 0x3F00) {
        address = address & 0x0FFF;
        if (ppu->mirroring == HORIZONTAL) {
            if (address >= 0x0800) {
                address -= 0x0800;
            }
            if (address >= 0x0400) {
                address -= 0x0400;
            }
        } else if (ppu->mirroring == VERTICAL) {
            if (address >= 0x0800) {
                address -= 0x0800;
            }
            if (address >= 0x0400 && address < 0x0800) {
                address -= 0x0400;
            }
        } else if (ppu->mirroring == SINGLE_SCREEN_LOWER) {
            address = address & 0x03FF;
        } else if (ppu->mirroring == SINGLE_SCREEN_UPPER) {
            address = (address & 0x03FF) + 0x0400;
        }
        return ppu->vram[address];
    } else if (address < 0x4000) {
        address = address & 0x1F;
        if (address & 0x03 == 0x00) {
            address = 0x00;
        }
        return ppu->palette[address];
    }
    return 0;  // Default return for invalid reads
}

void ppu_write(PPU* ppu, uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        ppu->bus->cart.mapper->write_chr(&ppu->bus->cart, address & 0x1FFF, value);
    } else if (address < 0x3F00) {
        uint16_t mirrored = (address - 0x2000) & 0x0FFF;
        if (ppu->mirroring == VERTICAL) {
            // Write to both vertical-mirrored regions
            ppu->vram[(mirrored & 0x07FF)] = value;  // Base region
            ppu->vram[(mirrored & 0x07FF) + 0x0800] = value;  // Mirrored region
        } else if (ppu->mirroring == HORIZONTAL) {
            // Write to both horizontal-mirrored regions
            ppu->vram[(mirrored & 0x03FF)] = value;  // Base region
            ppu->vram[(mirrored & 0x03FF) + 0x0400] = value;  // Mirrored region
        } else if (ppu->mirroring == SINGLE_SCREEN_LOWER) {
            // Everything mirrors to lower nametable
            ppu->vram[(mirrored & 0x03FF)] = value;
        } else if (ppu->mirroring == SINGLE_SCREEN_UPPER) {
            // Everything mirrors to upper nametable
            ppu->vram[(mirrored & 0x03FF) + 0x0400] = value;
        }
    } else if (address < 0x4000) {
        address = address & 0x1F;
        if (address == 0x10) {
            address = 0x00;
        }
        ppu->palette[address] = value;
    }
}

void ppu_step(PPU* ppu) {
    if (ppu->scanline >= 0 && ppu->scanline < 240) {
        if (ppu->cycle == 0) {
            evaluate_sprites(ppu);
        } else if (ppu->cycle == 340) {
            if (ppu->ppumask.BG_RENDER == 1) {
                render_scanline(ppu);
            }
            if (ppu->ppumask.SP_RENDER == 1) {
                render_sprites(ppu);
            }
            if (ppu->ppumask.BG_RENDER == 1 || ppu->ppumask.SP_RENDER == 1) {
                memcpy(&ppu->framebuffer[(ppu->scanline * 256)], &ppu->pixel_buffer[0], 256);
            }
        }
    } else if (ppu->scanline == 240) {
    } else if (ppu->scanline == 241) {
        if (ppu->cycle == 1) {
            ppu->ppustatus.VBLANK = 1;
            ppu_trigger_nmi(ppu);
            ppu->vblank_triggered = 1;
        }
    } else if (ppu->scanline == 261) {
        if (ppu->cycle == 0) {
            //evaluate_sprites(ppu);
            ppu->frame_odd = !ppu->frame_odd;
        } else if (ppu->cycle == 1) {
            ppu->ppustatus.VBLANK = 0;
            ppu->ppustatus.SP0_HIT = 0;
            memset(&ppu->framebuffer, 0, (0xF000));
            ppu->v.value = ppu->t.value;
        }
    }
    ppu->cycle += 1;
    if (ppu->cycle >= 341) {
        ppu->cycle = 0;
        ppu->scanline += 1;
        if (ppu->scanline >= 262) {
            ppu->frame_count += 1;
            ppu->scanline = 0;
            ppu->vblank_triggered = 0;
        }
    }
}

void ppu_trigger_nmi(PPU* ppu) {
    if (ppu->bus && ppu->ppuctrl.NMI_VBL==1) {
        bus_trigger_nmi(ppu->bus);
    }
}