#include "bus.h"

static inline uint8_t flip_byte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void renderer_init(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    memset(renderer,0, sizeof(Renderer));
    memset(renderer->secondary_oam.raw,0xFF,0x20);
}

void renderer_step(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    if(renderer->scanline < 240) {
        render_visible_scanline(ppu);
    } else if (renderer->scanline == 240) {
        // fuck all
    } else if (renderer->scanline == 241 && renderer->cycle == 1) {
        ppu->ppustatus.VBLANK = 1;
        ppu_trigger_nmi(ppu);
        render_rgb(ppu);
        ppu->vblank_triggered = 1;
    } else if (renderer->scanline == 261) {
        render_prerender_scanline(ppu);
    }
    renderer->cycle++;
    if (renderer->cycle > 340) {
        renderer->cycle = 0;
        renderer->scanline += 1;
        if (renderer->scanline > 261) {
            renderer->frame_count += 1;
            renderer->scanline = 0;
            renderer->frame_odd = !renderer->frame_odd;
        }
    }
}

void render_visible_scanline(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint16_t cycle = renderer->cycle;
    if(cycle == 0) {
        // pass
    }
    // Rendering
    if((cycle >=1 && cycle <= 256) || (cycle >= 321 && cycle <= 336)) {
        update_shifters(ppu);
        // BG rendering
        switch((cycle - 1) % 8) {
            case 0:
                load_shifters(ppu);
                read_NT(ppu);
                break;
            case 2:
                read_AT(ppu);
                break;
            case 4:
                read_BG_lsb(ppu);
                break;
            case 6:
                read_BG_msb(ppu);
                break;
            case 7:
                inc_hori(ppu);
                break;
            default:
                break;
        }
        // clear secondary OAM
        if(cycle >1 && cycle <=64) {
            uint8_t offset = (cycle-1) >> 1;
            if((cycle % 2)==1) {
                renderer->secondary_oam.raw[offset] = 0xFF;
            }
        }
        render_pixel(ppu);
    }
    if(cycle == 256) {
        inc_vert(ppu);
    }
    if(cycle >= 257 && cycle <= 320) {
        if(cycle == 257) {
            reset_hori(ppu);
            eval_sprite(ppu);
            renderer->sprite_zero_rendered = false;
        }
        if((cycle % 8)==0) {
            uint8_t sprite_id = ((cycle-264) >> 3);
            if(sprite_id < renderer->sprite_count) {
                fetch_sprite(ppu, sprite_id);
            } else {
                renderer->sprite_shifter_pattern_low[sprite_id] = 0;
                renderer->sprite_shifter_pattern_high[sprite_id] = 0;
                renderer->sprite_x_counters[sprite_id] = 255;
            }
        }
    }
}

void render_prerender_scanline(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint16_t cycle = renderer->cycle;
    if(cycle == 1) {
        ppu->ppustatus.VBLANK = 0;
        ppu->ppustatus.SP0_HIT = 0;
        ppu->ppustatus.SP_OVERFLOW = 0;
    }
    if(cycle >= 257 && cycle <= 320) {
        if(cycle == 257) {
            load_shifters(ppu);
            reset_hori(ppu);
        }
        if(cycle >=280 && cycle <= 304) {
            reset_vert(ppu);
        }
    }
    if(cycle == 261) {
        for (int i = 0; i < 32; i++) {
            renderer->secondary_oam.raw[i] = 0xFF;
        }
        for (int i = 0; i < 8; i++) {
            renderer->sprite_shifter_pattern_low[i] = 0;
            renderer->sprite_shifter_pattern_high[i] = 0;
            renderer->sprite_attributes[i].value = 0;
            renderer->sprite_x_counters[i] = 0xFF;
        }
        renderer->sprite_count = 0;
        renderer->sprite_zero_rendered = false;
    }
    if((cycle >= 321 && cycle <= 336)) {
        update_shifters(ppu);
        // BG rendering
        switch((cycle - 1) % 8) {
            case 0:
                load_shifters(ppu);
                read_NT(ppu);
                break;
            case 2:
                read_AT(ppu);
                break;
            case 4:
                read_BG_lsb(ppu);
                break;
            case 6:
                read_BG_msb(ppu);
                break;
            case 7:
                inc_hori(ppu);
                break;
            default:
                break;
        }
    }
}

void render_pixel(PPU *ppu) {
    Renderer* renderer = &ppu->renderer;
    int x = renderer->cycle - 1;   // pixel position (0–255)
    int y = renderer->scanline;    // current scanline (0–239)
    uint8_t bg_pixel = 0;
    uint8_t bg_palette = 0;
    uint8_t spr_pixel = 0;
    uint8_t spr_palette = 0;
    bool spr_priority = false;
    if (ppu->ppumask.BG_RENDER) {
        uint16_t bit_mux = 0x8000 >> renderer->x;
        uint8_t p0 = (renderer->bg_shifter_pattern_low & bit_mux) ? 1 : 0;
        uint8_t p1 = (renderer->bg_shifter_pattern_high & bit_mux) ? 1 : 0;
        bg_pixel = (p1 << 1) | p0;
        uint8_t a0 = (renderer->bg_shifter_attribute_low & bit_mux) ? 1 : 0;
        uint8_t a1 = (renderer->bg_shifter_attribute_high & bit_mux) ? 1 : 0;
        bg_palette = (a1 << 1) | a0;
    }
    if (ppu->ppumask.SP_RENDER) {
        for (int i = 0; i < renderer->sprite_count; i++) {
            if (renderer->sprite_x_counters[i] == 0) {
                uint8_t p0 = (renderer->sprite_shifter_pattern_low[i] & 0x80) >> 7;
                uint8_t p1 = (renderer->sprite_shifter_pattern_high[i] & 0x80) >> 6;
                uint8_t pixel = p1 | p0;
                if (pixel != 0) { // Non-transparent sprite pixel
                    spr_pixel = pixel;
                    spr_palette = renderer->sprite_attributes[i].palette;
                    spr_priority = (renderer->sprite_attributes[i].priority == 0);
                    // Set sprite zero hit flag
                    if (renderer->sprite_zero_rendered && i == 0 && bg_pixel != 0 && renderer->cycle < 256) {
                        ppu->ppustatus.SP0_HIT = true;
                    }
                    break; // first opaque sprite pixel found, stop checking
                }
            }
        }
    }
    uint8_t final_pixel = 0;
    uint8_t final_palette = 0;
    if (bg_pixel == 0 && spr_pixel == 0) { // Both transparent; use universal background color
        final_pixel = 0;
        final_palette = 0;
    } else if (bg_pixel == 0 && spr_pixel > 0) { // BG transparent, Sprite opaque
        final_pixel = spr_pixel;
        final_palette = spr_palette + 4;
    } else if (bg_pixel > 0 && spr_pixel == 0) { // BG opaque, Sprite transparent
        final_pixel = bg_pixel;
        final_palette = bg_palette;
    } else { // Both opaque
        if (spr_priority) {
            final_pixel = spr_pixel;
            final_palette = spr_palette + 4;
        } else {
            final_pixel = bg_pixel;
            final_palette = bg_palette;
        }
    }
    uint16_t color_address = 0x3F00 | (final_palette << 2) | final_pixel;
    uint8_t color_index = ppu_read(ppu, color_address);
    renderer->framebuffer[(y * 256) + x] = color_index;
}

void load_shifters(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    renderer->bg_shifter_pattern_low = (renderer->bg_shifter_pattern_low & 0xFF00) | renderer->bg_next_pattern_lsb;
    renderer->bg_shifter_pattern_high = (renderer->bg_shifter_pattern_high & 0xFF00) | renderer->bg_next_pattern_msb;
    renderer->bg_shifter_attribute_low = (renderer->bg_shifter_attribute_low & 0xFF00) | ((renderer->bg_next_attr & 0x01) ? 0xFF : 0x00);
    renderer->bg_shifter_attribute_high = (renderer->bg_shifter_attribute_high & 0xFF00) | ((renderer->bg_next_attr & 0x02) ? 0xFF : 0x00);
}

void update_shifters(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    if (ppu->ppumask.BG_RENDER == 1) {
        renderer->bg_shifter_pattern_low <<= 1;
        renderer->bg_shifter_pattern_high <<= 1;
        renderer->bg_shifter_attribute_low <<= 1;
        renderer->bg_shifter_attribute_high <<= 1;
    }
    if ((ppu->ppumask.SP_RENDER == 1) && (renderer->cycle >=1 && renderer->cycle <= 256)) {
        for(int i=0; i < renderer->sprite_count; i++) {
            if (renderer->sprite_x_counters[i] == 0) {
                renderer->sprite_shifter_pattern_low[i] <<= 1;
                renderer->sprite_shifter_pattern_high[i] <<= 1;
            } else if(renderer->sprite_x_counters[i] > 0) {
                renderer->sprite_x_counters[i]--;
            }
        }
    }
}

void read_NT(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    renderer->bg_next_tile_id = ppu_read(ppu, 0x2000 | (renderer->v.value & 0x0FFF));
}

void read_AT(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint16_t attr_address = 0x23C0 
    | (renderer->v.nametable_y << 11)
    | (renderer->v.nametable_x << 10)
    | ((renderer->v.coarse_y >> 2) << 3)
    | (renderer->v.coarse_x >> 2);
    renderer->bg_next_attr = ppu_read(ppu, attr_address);
    if(renderer->v.coarse_y & 0x02) renderer->bg_next_attr >>= 4;
    if(renderer->v.coarse_x & 0x02) renderer->bg_next_attr >>= 2;
}

void read_BG_lsb(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint16_t address = (ppu->ppuctrl.BGTABLE << 12) + ((uint16_t)renderer->bg_next_tile_id << 4) + (renderer->v.fine_y) + 0;
    renderer->bg_next_pattern_lsb = ppu_read(ppu, address);
}

void read_BG_msb(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint16_t address = (ppu->ppuctrl.BGTABLE << 12) + ((uint16_t)renderer->bg_next_tile_id << 4) + (renderer->v.fine_y) + 8;
    renderer->bg_next_pattern_msb = ppu_read(ppu, address);
}

void inc_hori(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    if(ppu->ppumask.BG_RENDER || ppu->ppumask.SP_RENDER) {
        if(renderer->v.coarse_x == 31) {
            renderer->v.coarse_x = 0;
            renderer->v.nametable_x = ~renderer->v.nametable_x;
        } else {
            renderer->v.coarse_x++;
        }
    }
}

void inc_vert(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    if(ppu->ppumask.BG_RENDER || ppu->ppumask.SP_RENDER) {
        if(renderer->v.fine_y < 7) {
            renderer->v.fine_y++;
        } else {
            renderer->v.fine_y = 0;
            if(renderer->v.coarse_y == 29) {
                renderer->v.coarse_y = 0;
                renderer->v.nametable_y = ~renderer->v.nametable_y;
            } else if(renderer->v.coarse_y == 31) {
                    renderer->v.coarse_y = 0;
            } else {
                renderer->v.coarse_y++;
            }
        }
    }
}

void reset_hori(PPU* ppu){
    Renderer* renderer = &ppu->renderer;
    if(ppu->ppumask.BG_RENDER || ppu->ppumask.SP_RENDER) {
        renderer->v.nametable_x = renderer->t.nametable_x;
        renderer->v.coarse_x = renderer->t.coarse_x;
    }
}

void reset_vert(PPU* ppu){
    Renderer* renderer = &ppu->renderer;
    if(ppu->ppumask.BG_RENDER || ppu->ppumask.SP_RENDER) {
        renderer->v.fine_y = renderer->t.fine_y;
        renderer->v.nametable_y = renderer->t.nametable_y;
        renderer->v.coarse_y = renderer->t.coarse_y;
    }
}

void eval_sprite(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint8_t sprite_height = (ppu->ppuctrl.SPRITESIZE == 1 ? 16 : 8);

    renderer->sprite_count = 0;
    renderer->sprite_overflow = false;

    for (int sprite_id = 0; sprite_id < 64; sprite_id++) {
        Sprite sprite = ppu->oam.sprites[sprite_id];
        int diff = (renderer->scanline + 1) - sprite.y;
        if (diff >= 0 && diff < sprite_height) {
            if (renderer->sprite_count < 8) {
                renderer->secondary_oam.sprites[renderer->sprite_count++] = sprite;
            } else {
                renderer->sprite_overflow = true;
                break;  // 8 sprites found, overflow detected
            }
        }
    }
}

void fetch_sprite(PPU* ppu, int id) {
    Renderer* renderer = &ppu->renderer;
    int sprite_height = ppu->ppuctrl.SPRITESIZE == 1 ? 16 : 8;
    Sprite sprite = renderer->secondary_oam.sprites[id];
    int row = (renderer->scanline + 1) - sprite.y;
    if(sprite.attr.flip_v) {
        row = (sprite_height -1) - row;
    }
    uint16_t addr;
    if(sprite_height == 8) {
        uint16_t base = ppu->ppuctrl.SPRITETABLE == 1 ? 0x1000: 0x0000;
        addr = base + (sprite.tile_id * 16) + row;
    } else {
        uint16_t base = ((sprite.tile_id & 0x01) == 1 ? 0x1000: 0x0000);
        uint8_t tile_id = sprite.tile_id & 0xFE;
        if(row > 7) {
            tile_id++;
            row -= 8;
        }
        addr = base + (tile_id * 16) + row;
    }
    uint8_t low_byte = ppu_read(ppu, addr);
    uint8_t high_byte = ppu_read(ppu, addr + 8);
    if (sprite.attr.flip_h) {
        low_byte = flip_byte(low_byte);
        high_byte = flip_byte(high_byte);
    }
    renderer->sprite_shifter_pattern_low[id] = low_byte;
    renderer->sprite_shifter_pattern_high[id] = high_byte;
    renderer->sprite_attributes[id] = sprite.attr;
    renderer->sprite_x_counters[id] = sprite.x;
    renderer->sprite_zero_rendered = (id ==0 && renderer->secondary_oam.sprites[0].tile_id == ppu->oam.sprites[0].tile_id);
}

void render_rgb(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint8_t *indexed_pixels = renderer->framebuffer;
    uint8_t *rgb_pixels = renderer->framebuffer_rgb;
    uint8_t *palette = ppu->colors;  // Pointer to palette (faster access)
    for (int i = 0; i < 256 * 240; i++) {
        uint8_t *color = &palette[indexed_pixels[i] * 3];  // Direct lookup
        rgb_pixels[i * 3] = color[0];  // R
        rgb_pixels[i * 3 + 1] = color[1];  // G
        rgb_pixels[i * 3 + 2] = color[2];  // B
    }
}
