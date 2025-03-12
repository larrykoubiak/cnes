#include "bus.h"

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
    uint16_t nametable_address, attribute_address, tile_address, palette_address;
    uint8_t nametable_byte, attribute_byte, tile_low_byte, tile_high_byte;
    LOOPYRegister v_copy = ppu->v;
    fine_x = ppu->x;
    for (x=0; x<256;x++){
        // fetch nametable
        nametable_address = 0x2000 | (v_copy.value & 0x0FFF);
        nametable_byte = ppu_read(ppu, nametable_address);
        // fetch attribute
        attribute_address = 0x23C0 | (v_copy.nametable << 10) | ((v_copy.coarse_y >> 2) << 3) | (v_copy.coarse_x >> 2);
        attribute_byte = ppu_read(ppu, attribute_address);
        // fetch tile
        tile_address = (ppu->ppuctrl.BGTABLE << 12) | (nametable_byte << 4) | v_copy.fine_y;
        tile_low_byte = ppu_read(ppu, tile_address);
        tile_high_byte = ppu_read(ppu, tile_address + 8);
        pixel = ((tile_low_byte >> (7 - fine_x)) & 1) | (((tile_high_byte >> (7 - fine_x)) & 1) << 1);
        quadrant = (((v_copy.coarse_y >> 1) & 1) << 1) | ((v_copy.coarse_x >> 1) & 1);
        palette_index = (attribute_byte >> (quadrant * 2)) & 0x03;
        palette_address = 0x3F00 + (palette_index << 2) + pixel;
        color = ppu_read(ppu, palette_address);
        ppu->pixel_buffer[x] = color;
        if (fine_x == 7) {
            fine_x = 0;
            if (v_copy.coarse_x == 31) {
                v_copy.coarse_x = 0;
                v_copy.nametable ^= 1; // Switch horizontal nametable
            } else {
                v_copy.coarse_x += 1;
            }
        } else {
            fine_x += 1;
        }
    }
    if (ppu->v.fine_y < 7) {
        ppu->v.fine_y += 1;
    } else {
        ppu->v.fine_y = 0;
        if (ppu->v.coarse_y == 29) {
            ppu->v.coarse_y = 0;
            ppu->v.nametable ^= 2;
        } else if (ppu->v.coarse_y == 31) {
            ppu->v.coarse_y = 0; // Wrap around (needed for scrolling)
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
    memset(&ppu->secondary_oam,0xFF,sizeof(Sprite)*8);
    for (i=0; i<64; i++) {
        sprite = ppu->oam.sprites[i];
        yrange = ppu->scanline - sprite.y;
        if (yrange >=0 && yrange < sprite_height && ppu->sprite_count < 8) {
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
    uint16_t palette_address;
    Sprite sprite;
    int sprite_height = ppu->ppuctrl.SPRITESIZE == 0 ? 8 : 16;
    for (i=0; i<ppu->sprite_count; i++) {
        sprite = ppu->secondary_oam[i];
        if(sprite.y > 240) {
            continue;
        }
        y_offset = ppu->scanline - sprite.y;
        if (ppu->ppuctrl.SPRITESIZE == 0) {
            tile_address = ((ppu->ppuctrl.SPRITETABLE << 12) | (sprite.tile_id << 4) | (sprite.attr.flip_v ? 7 - y_offset: y_offset));
        } else {
            uint16_t bank = (sprite.tile_id& 0x01) << 12;
            uint8_t tile_index = sprite.tile_id & 0xFE;
            if (sprite.attr.flip_v) {
                tile_address = (y_offset < 8)
                    ? bank | ((tile_index +1) << 4) | (7 - (y_offset & 7))
                    : bank | (tile_index  << 4) | (7 - (y_offset & 7));
            } else {
                tile_address = (y_offset < 8)
                    ? bank | ((tile_index +1) << 4) | (y_offset & 7)
                    : bank | (tile_index  << 4) | (y_offset & 7);
            }
        }
        sprite_tile_low = ppu_read(ppu, tile_address);
        sprite_tile_high = ppu_read(ppu, tile_address + 8);
        for (x=0; x<8; x++) {
            if (sprite.x + x >= 256) {
                break;
            }
            int bit_pos = sprite.attr.flip_h ? x : (7 - x);
            pixel = ((sprite_tile_low >> bit_pos) & 1) | (((sprite_tile_high >> bit_pos) & 1) << 1);
            if (pixel == 0) {
                continue;
            }
            palette_index = (sprite.attr.palette & 0x03) + 4;
            palette_address = 0x3F10 + (palette_index << 2) + pixel;
            color = ppu_read(ppu, palette_address);
            if (sprite.attr.priority == 0 || ppu->pixel_buffer[sprite.x + x] == 0) {
                ppu->pixel_buffer[sprite.x + x] = color;
            }
            if (i == 0 && ppu->scanline < 240 && sprite.x + x < 256) {
                if (ppu->pixel_buffer[sprite.x + x] != 0 && pixel != 0) {
                    ppu->ppustatus.SP0_HIT = 1;
                }
            }
        }
    }
}