#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cartridge.h"
#include "ppu.h"
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

void render_scanline(PPU* ppu) {
    int x;
    uint8_t pixel, palette_index, color, fine_x, quadrant;
    uint16_t nametable_address, attribute_address, tile_address;
    uint8_t nametable_byte, attribute_byte, tile_low_byte, tile_high_byte;
    LOOPYRegister v_copy = ppu->v;
    fine_x = ppu->x;
    for (x=0; x<256;x++){
        // fetch nametable
        nametable_address = 0x2000 | (v_copy.nametable << 10) | ((v_copy.coarse_y & 0x1F) << 5) | (v_copy.coarse_x & 0x1F);
        nametable_byte = ppu_read(ppu, nametable_address);
        // fetch attribute
        attribute_address = 0x2000 | (v_copy.nametable << 10) | 0x3C0 | ((v_copy.coarse_y >> 2) << 3) | (v_copy.coarse_x >> 2);
        attribute_byte = ppu_read(ppu, attribute_address);
        // fetch tile
        tile_address = (ppu->ppuctrl.BGTABLE << 12) | (nametable_byte << 4) | v_copy.fine_y;
        tile_low_byte = ppu_read(ppu, tile_address);
        tile_high_byte = ppu_read(ppu, tile_address + 8);
        pixel = ((tile_low_byte >> (7 - fine_x)) & 1) | (((tile_high_byte >> (7 - fine_x)) & 1) << 1);
        quadrant = (((v_copy.coarse_y >> 1) & 1) << 1) | ((v_copy.coarse_x >> 1) & 1);
        palette_index = (attribute_byte >> (quadrant * 2)) & 0x03;
        color = ppu->palette[(palette_index << 2) | pixel];
        ppu->pixel_buffer[x] = color;
        fine_x += 1;
        if (fine_x == 8) {
            fine_x = 0;
            if (v_copy.coarse_x == 31) {
                v_copy.coarse_x = 0;
                v_copy.nametable ^= 1;
            } else {
                v_copy.coarse_x += 1;
            }
        }
    }
    if (ppu->v.fine_y < 7) {
        ppu->v.fine_y += 1;
    } else {
        ppu->v.fine_y = 0;
        if (ppu->v.coarse_y == 29) {
            ppu->v.coarse_y = 0;
            ppu->v.nametable ^= 2;
        } else {
            ppu->v.coarse_y += 1;
        }
    }
}

void evaluate_sprites(PPU* ppu) {
    int i, yrange;
    uint8_t sprite_height;
    Sprite sprite;
    sprite_height = 8 << ppu->ppuctrl.SPRITESIZE;
    ppu->sprite_count = 0;
    for (i=0; i<64; i++) {
        sprite = ppu->oam.sprites[i];
        yrange = ppu->scanline - sprite.y;
        if (yrange >=0 && yrange < sprite_height) {
            if (ppu->sprite_count < 8) {
                ppu->secondary_oam[ppu->sprite_count] = sprite;
                ppu->sprite_count += 1;
            } else {
                ppu->ppustatus.SP_OVERFLOW = 1;
                break;
            }
        }
    }
}

void render_sprites(PPU* ppu) {
    int i, x, pixel, palette_index, color, tile_address, y_offset;
    uint8_t sprite_tile_low, sprite_tile_high;
    Sprite sprite;
    int sprite_height = ppu->ppuctrl.SPRITESIZE == 0 ? 8 : 16;
    for (i=0; i<ppu->sprite_count; i++) {
        sprite = ppu->secondary_oam[i];
        if(sprite.y > 240) {
            continue;
        }
        y_offset = ppu->scanline - sprite.y;
        if (sprite.attr.flip_v) {
            y_offset = (sprite_height - 1) - y_offset;
        }
        tile_address = ((ppu->ppuctrl.SPRITETABLE << 12) | (sprite.tile_id * 16) | y_offset);
        sprite_tile_low = ppu_read(ppu, tile_address);
        sprite_tile_high = ppu_read(ppu, tile_address + 8);
        for (x=0; x<8; x++) {
            if (sprite.x + x >= 256) {
                break;
            }
            pixel = ((sprite_tile_low >> (7 - x)) & 1) | \
                    (((sprite_tile_high >> (7 - x)) & 1) << 1);
            if (pixel == 0) {
                continue;
            }
            palette_index = (sprite.attr.palette & 0x03) + 4;
            color = ppu->palette[(palette_index << 2) | pixel];
            if (sprite.attr.priority == 0 || ppu->pixel_buffer[sprite.x + x] == 0) {
                ppu->pixel_buffer[sprite.x + x] = color;
            }
            if (i == 0 && ppu->scanline < 240 && sprite.x + x < 256) {
                if (ppu->pixel_buffer[sprite.x + x] != 0) {
                    ppu->ppustatus.SP0_HIT = 1;
                }
            }
        }
    }
}

void render_tile(PPU* ppu, uint8_t tile_id, uint8_t palette_id, int palette_table_id, int pattern_table_id, uint8_t* output_buffer ) {
    int x, y, c, bit0, bit1, color_index, pixel_index;
    uint16_t base_address, tile_address, color_addr;
    uint8_t* pixel;
    uint8_t low_byte, high_byte;
    base_address = (pattern_table_id & 0x01) << 12;
    tile_address = base_address + (tile_id * 16);
    for (y=0; y<8; y++){
        low_byte = ppu_read(ppu, tile_address + y);       // Plane 0
        high_byte = ppu_read(ppu, tile_address + y + 8);  // Plane 1
        for (x=0; x<8; x++) {
            bit0 = (low_byte >> (7 - x)) & 0x01;
            bit1 = (high_byte >> (7 - x)) & 0x01;
            color_index = (bit1 << 1) | bit0;
            color_addr = get_color_address(ppu, palette_table_id, palette_id, color_index);
            pixel_index = ((y * 8) + x) * 3;
            for (c=0; c<3; c++) {
                output_buffer[pixel_index + c] = ppu->colors[color_addr + c];
            }
        }
    }
}

void render_nametable(PPU* ppu, uint8_t nametable_id, uint8_t* output_buffer)  {
    int x, y, tx, ty, c, tile_index, pixel_index, tile_pixel, palette_id, palette_shift;
    uint8_t attr_byte;
    uint8_t tile_buffer[8 * 8 * 3];
    bool pattern_table_id;
    uint16_t nametable_address;
    uint16_t attr_table_address;
    pattern_table_id = ppu->ppuctrl.BGTABLE;
    nametable_address = 0x2000 + (nametable_id << 10);
    attr_table_address = (nametable_address + 0x3C0);
    for (y=0; y<30; y++) {
        for (x=0; x <32; x++) {
            tile_index = ppu_read(ppu, nametable_address + (y * 32) + x);
            tx, ty = x / 4, y / 4;
            attr_byte = ppu_read(ppu, attr_table_address + (ty * 8) + tx);
            palette_shift = ((y % 4) / 2) * 4 + ((x % 4) / 2) * 2;
            palette_id = (attr_byte >> palette_shift) & 0x03;
            render_tile(ppu, tile_index, palette_id, 0, pattern_table_id, (uint8_t*)tile_buffer);
            for (ty=0; ty < 8; ty++) {
                for (tx=0; tx <8;tx++){
                    pixel_index = ((((y * 8) + ty) * 256) + ((x * 8) + tx)) * 3;
                    tile_pixel = ((ty * 8) + tx) * 3;
                    for (c=0; c< 3; c++) {
                        output_buffer[pixel_index + c] = tile_buffer[tile_pixel + c];
                    }
                }
            }
        }
    }
}

void render_patterntable(PPU* ppu, int table_id, uint8_t* output_buffer) {
    int x, y, c, tx, ty, palette_table_id, tile_index, pixel_index, tile_pixel;
    uint8_t tile_buffer[8 * 8 * 3];
    palette_table_id = ~table_id & 0x01;
    for (y=0; y<16; y++) {
        for (x=0; x < 16; x++) {
            tile_index = (y * 16) + x;
            render_tile(ppu, tile_index, 0, palette_table_id, table_id, (uint8_t*)tile_buffer);
            for (ty=0; ty <8; ty++) {
                for (tx=0; ty < 8; ty++) {
                    pixel_index = ((((y * 8) + ty) * 128) + (x * 8 + tx)) * 3;
                    tile_pixel = ((ty * 8) + tx) * 3;
                    for (c=0; c<3; c++) {
                        output_buffer[pixel_index + c] = tile_buffer[tile_pixel + c];
                    }
                }
            }
        }
    }
}

void render_palettetable(PPU* ppu, uint8_t* output_buffer) {
    int pal_table_id, pal_id, color_id, pixel_index, c;
    uint16_t color_addr;
    for (pal_table_id=0; pal_table_id < 2; pal_table_id++) {
        for (pal_id=0;pal_id<4; pal_id++) {
            for (color_id=0; color_id <4;color_id++) {
                color_addr = get_color_address(ppu, pal_table_id, pal_id, color_id);
                pixel_index = ((pal_table_id * 16) + (pal_id * 4) + color_id) * 3;
                for (c=0; c <3; c++) {
                    output_buffer[pixel_index + c] = ppu->colors[color_addr + c];
                }
            }
        }
    }
}