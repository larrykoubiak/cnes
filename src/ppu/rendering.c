#include "../bus.h"

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

uint16_t get_color_address(PPU* ppu, int pal_table_id, uint8_t pal_id, uint8_t pixel ) {
    uint16_t pal_idx = 0x3F00 + (((pal_table_id & 0x01) << 4) | ((pal_id & 0x03) << 2) | (pixel & 0x03));
    uint8_t color_id = ppu_read(ppu, pal_idx);
    uint16_t color_address = (((ppu->ppumask.value & 0xE0) >> 5) * 0XC0) + (color_id * 3);
    return color_address;
}