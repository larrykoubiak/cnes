#include "bus.h"

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

static inline uint16_t map_nametable_address(uint16_t address, MirroringMode mode) {
    uint16_t offset = address & 0xFFF;
    uint16_t table = offset >> 10;
    uint16_t fine = offset & 0x03FF;
    switch(mode) {
        case HORIZONTAL:
            return (table < 2) ? fine : (fine + 0x400);
        case VERTICAL:
            return (table == 0 || table == 2) ? fine : (fine + 0x400);
        case SINGLE_SCREEN_LOWER:
            return fine;
        case SINGLE_SCREEN_UPPER:
            return fine + 0x400;
    }
}

int ppu_init(PPU* ppu, Bus* bus, MirroringMode mode) {
    memset(ppu, 0, sizeof(PPU));
    LoadColorsResult result = load_colors(ppu, "2C02G_wiki.pal");
    ppu->bus = bus;
    ppu->mirroring = mode;
}

void ppu_reset(PPU* ppu) {
    memset(&ppu->vram,0,0x1000);
    memset(&ppu->oam,0xFF,sizeof(Sprite)*64);
    memset(&ppu->palette,0,0x20);
    ppu->ppuctrl.value = 0;
    ppu->ppumask.value = 0;
    ppu->ppustatus.value = 0;
    ppu->open_bus_val = 0;
    ppu->oamaddr = 0;
    ppu->ppuaddr = 0;
    ppu->ppudata = 0;
}

uint8_t ppu_register_read(PPU* ppu, uint16_t address) {
    uint8_t result = 0;
    int increment = 0;
    switch(address) {
        case 2:
            result = (ppu->ppustatus.value & 0xE0) | (ppu->open_bus_val & 0x1F);
            ppu->ppustatus.VBLANK = 0;
            ppu->renderer.w = 0;
            break;
        case 4:
            result = ppu->oam.raw[ppu->oamaddr];
            break;
        case 7:
            result = ppu->open_bus_val;
            ppu->open_bus_val = ppu_read(ppu, ppu->renderer.v.value);
            if (ppu->renderer.v.value >= 0x3F00) {
                result = ppu->open_bus_val;
            }
            increment = ppu->ppuctrl.VRAM_DIRECTION == 1 ? 32 : 1;
            ppu->renderer.v.value += increment;
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
            ppu->renderer.t.nametable_x = ppu->ppuctrl.NAMETABLE_X;
            ppu->renderer.t.nametable_y = ppu->ppuctrl.NAMETABLE_Y;
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
            if (ppu->renderer.w == 0) {
                ppu->renderer.t_x = value & 0x07;
                ppu->renderer.t.coarse_x = (value >> 3) & 0x1F;
            } else {
                ppu->renderer.t.fine_y = value & 0x07;
                ppu->renderer.t.coarse_y = (value >> 3) & 0x1F;
            }
            ppu->renderer.w ^= 1;
            break;
        case 6:
            if (ppu->renderer.w == 0) {
                ppu->renderer.t.value = (ppu->renderer.t.value & 0x00FF) | (uint16_t)((value & 0x3F) << 8);
            } else {
                ppu->renderer.t.value = (ppu->renderer.t.value & 0xFF00) | value;
                ppu->renderer.v.value = ppu->renderer.t.value;
            }
            ppu->renderer.w ^= 1;
            break;
        case 7:
            ppu_write(ppu, ppu->renderer.v.value, value);
            increment = ppu->ppuctrl.VRAM_DIRECTION == 1 ? 32 : 1;
            ppu->renderer.v.value =(ppu->renderer.v.value + increment) & 0x3FFF;
        default:
            break;
    }
}

uint8_t ppu_read(PPU* ppu, uint16_t address) {
    if (address < 0x2000) {
        return ppu->bus->cart.mapper->read_chr(&ppu->bus->cart, address & 0x1FFF);
    } else if (address < 0x3F00) {
        uint16_t idx = map_nametable_address(address, ppu->mirroring);
        return ppu->vram[idx];
    } else if (address < 0x4000) {
        address = address & 0x1F;
        if ((address & 0x03) == 0x00) {
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
        uint16_t idx = map_nametable_address(address, ppu->mirroring);
        ppu->vram[idx] = value;
    } else if (address < 0x4000) {
        address = address & 0x1F;
        if (address == 0x10) {
            address = 0x00;
        }
        ppu->palette[address] = value;
    }
}

void ppu_trigger_nmi(PPU* ppu) {
    if (ppu->bus && ppu->ppuctrl.NMI_VBL==1) {
        bus_trigger_nmi(ppu->bus);
    }
}

void ppu_step(PPU* ppu) {
    renderer_step(ppu);
}