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
            render_rgb(ppu);
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
    uint8_t pixel, pixel_low, pixel_high, color, shift, mask;
    uint16_t nametable_address, attribute_address, tile_address, palette_address;
    uint8_t nametable_byte, attribute_byte, tile_low_byte, tile_high_byte;
    for (x=0; x<256;x++){
        // fetch nametable
        nametable_address = 0x2000 | (ppu->v.value & 0x0FFF);
        nametable_byte = ppu_read(ppu, nametable_address);
        // fetch attribute
        attribute_address = 0x23C0 | (ppu->v.nametable_y << 11) | (ppu->v.nametable_y << 10) |((ppu->v.coarse_y >> 2) << 3) | (ppu->v.coarse_x >> 2);
        attribute_byte = ppu_read(ppu, attribute_address);
        if(ppu->v.coarse_y & 0x02) {
            attribute_byte >> 4;
        };
        if(ppu->v.coarse_x & 0x02) {
            attribute_byte >> 2;
        }
        attribute_byte &= 0x03;
        // fetch tile
        tile_address = (ppu->ppuctrl.BGTABLE << 12) | (nametable_byte << 4) | ppu->v.fine_y;
        tile_low_byte = ppu_read(ppu, tile_address);
        tile_high_byte = ppu_read(ppu, tile_address + 8);
        shift = 7 - ((ppu->x + x) % 8);
        mask = 1 << shift;
        pixel_high = (tile_high_byte & mask) >> shift;
        pixel_low = (tile_low_byte & mask) >> shift;
        pixel = (pixel_high << 1) | pixel_low ;
        palette_address = 0x3F00 | (attribute_byte << 2) | pixel;
        color = ppu_read(ppu, palette_address);
        ppu->pixel_buffer[x] = color;
        if (((ppu->x + x) % 8) == 7) {
            if (ppu->v.coarse_x == 31) {
                ppu->v.coarse_x = 0;
                ppu->v.nametable_x ^= 1; // Switch horizontal nametable
            } else {
                ppu->v.coarse_x += 1;
            }
        }
    }
    ppu->v.coarse_x = ppu->t.coarse_x;
    ppu->v.nametable_x = ppu->t.nametable_x;
    if (ppu->v.fine_y < 7) {
        ppu->v.fine_y += 1;
    } else {
        ppu->v.fine_y = 0;
        if (ppu->v.coarse_y == 29) {
            ppu->v.coarse_y = 0;
            ppu->v.nametable_y ^= 1;
        } else if (ppu->v.coarse_y == 31) {
            ppu->v.coarse_y = 0;
            ppu->v.nametable_y ^= 1;
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
    // clear secondary OAM
    ppu->sprite_count = 0;
    for (i=0; i<8; i++) {
        ppu->secondary_oam[i].y = 0xFF;
        ppu->secondary_oam[i].tile_id = 0x0;
        ppu->secondary_oam[i].attr.value = 0x0;
        ppu->secondary_oam[i].x = 0xFF;
    }
    // evaluate
    for (i=0; i<64; i++) {
        sprite = ppu->oam.sprites[i];
        yrange = ppu->scanline - sprite.y;
        if (yrange >=0 && yrange < sprite_height && ppu->sprite_count < 8) {
            if (ppu->sprite_count < 9) {
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
    int i, x, tile_address, y_offset;
    uint8_t pixel, pixel_low, pixel_high, color, shift, mask;
    uint8_t sprite_tile_low, sprite_tile_high;
    uint16_t palette_address;
    Sprite sprite;
    int sprite_height = ppu->ppuctrl.SPRITESIZE == 0 ? 8 : 16;
    for (i=0; i<ppu->sprite_count; i++) {
        sprite = ppu->secondary_oam[i];
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
            shift = sprite.attr.flip_h ? x : (7 - x);
            mask = 1 << shift;
            pixel_low = (sprite_tile_low & mask) >> shift;
            pixel_high = (sprite_tile_high & mask) >> shift;
            pixel = (pixel_high << 1) | pixel_low;
            if (pixel == 0) {
                continue;
            }
            palette_address = 0x3F10 | ((sprite.attr.palette & 0x03) << 2) | pixel;
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

void render_rgb(PPU* ppu) {
    // uint8_t color_idx;
    // int src_offset, dst_offset;
    // for(int y=0; y<240; y++) {
    //     for(int x=0; x<256;x++) {
    //         src_offset = ((y*256)+x);
    //         color_idx = ppu->framebuffer[src_offset] * 3;
    //         dst_offset = src_offset * 3;
    //         ppu->framebuffer_rgb[dst_offset + 0] = ppu->colors[color_idx + 0]; //r
    //         ppu->framebuffer_rgb[dst_offset + 1] = ppu->colors[color_idx + 1]; //g
    //         ppu->framebuffer_rgb[dst_offset + 2] = ppu->colors[color_idx + 2]; //b
    //     }
    // }
    uint8_t *indexed_pixels = ppu->framebuffer;
    uint8_t *rgb_pixels = ppu->framebuffer_rgb;
    uint8_t *palette = ppu->colors;  // Pointer to palette (faster access)
    for (int i = 0; i < 256 * 240; i++) {
        uint8_t *color = &palette[indexed_pixels[i] * 3];  // Direct lookup
        rgb_pixels[i * 3] = color[0];  // R
        rgb_pixels[i * 3 + 1] = color[1];  // G
        rgb_pixels[i * 3 + 2] = color[2];  // B
    }
}