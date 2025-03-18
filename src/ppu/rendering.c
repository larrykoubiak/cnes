#include "bus.h"

void renderer_init(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    memset(renderer,0, sizeof(Renderer));
    memset(renderer->secondary_oam,0xFF,0x20);
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
        //load x counters for sprites
        for(int i=0;i<8;i++) {
            renderer->sprite_x_counters[i] = renderer->secondary_oam->sprites[i].x;
        }
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
        // FG rendering
        if(cycle >1 && cycle <=64) {
            uint8_t offset = (cycle-1) >> 1;
            if(((cycle-1) % 2)==1) {
                renderer->secondary_oam->raw[offset] = 0xFF;
            }
        }
        if(cycle >64 && cycle <= 256) {
            if(cycle==65) {
                renderer->oam_m = 0;
                renderer->oam_n = 0;
                renderer->sec_oam_index = 0;
                renderer->sprite_count = 0;
                renderer->sprite_eval_done = false;
                renderer->sprite_overflow = false;
            }
            eval_sprite(ppu);
        }
        // TODO: Render pixel
    }
    if(cycle == 256) {
        inc_vert(ppu);
    }
    if(cycle >= 257 && cycle <= 320) {
        if(cycle == 257) {
            load_shifters(ppu);
            reset_hori(ppu);
        }
        // TODO : Fill sprite shifters
    }
}

void render_prerender_scanline(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    uint16_t cycle = renderer->cycle;
    if(cycle == 0) {
        //load x counters for sprites
        for(int i=0;i<8;i++) {
            renderer->sprite_x_counters[i] = renderer->secondary_oam->sprites[i].x;
        }
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
        // FG rendering
        if(cycle >1 && cycle <=64) {
            uint8_t offset = (cycle-1) >> 1;
            if(((cycle-1) % 2)==1) {
                renderer->secondary_oam->raw[offset] = 0xFF;
            }
        }
        if(cycle >64 && cycle <= 256) {
            // Sprite evaluation

        }
    }
    if(cycle == 256) {
        inc_vert(ppu);
    }
    if(cycle == 257) {
        load_shifters(ppu);
        reset_hori(ppu);
    }
}

void render_pixel(PPU* ppu) {

}

void load_shifters(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    renderer->bg_shifter_pattern_low = (renderer->bg_shifter_pattern_low & 0xFF00) | renderer->bg_next_pattern_lsb;
    renderer->bg_shifter_pattern_high = (renderer->bg_shifter_pattern_high & 0xFF00) | renderer->bg_next_pattern_msb;
    renderer->bg_shifter_attribute_low = (renderer->bg_shifter_pattern_high & 0xFF00) | (renderer->bg_next_attr & 0x01) ? 0xFF : 0x00;
    renderer->bg_shifter_attribute_high = (renderer->bg_shifter_attribute_high & 0xFF00) | (renderer->bg_next_attr & 0x02) ? 0xFF : 0x00;
}

void update_shifters(PPU* ppu) {
    Renderer* renderer = &ppu->renderer;
    if (ppu->ppumask.BG_RENDER == 1) {
        renderer->bg_shifter_pattern_low << 1;
        renderer->bg_shifter_pattern_high << 1;
        renderer->bg_shifter_attribute_low << 1;
        renderer->bg_shifter_attribute_high << 1;
    }
    if ((ppu->ppumask.SP_RENDER == 1) && (renderer->cycle >=1 && renderer->cycle <= 256)) {
        for(int i=0; i < 8; i++) {
            if(renderer->sprite_x_counters[i] > 0) {
                renderer->sprite_x_counters[i]--;
            } else {
                renderer->sprite_shifter_pattern_low[i] << 1;
                renderer->sprite_shifter_pattern_high[i] << 1;
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
    renderer->bg_next_attr = ppu_read(ppu, 0x23C0 | (renderer->v.value & 0x0FFF));
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
    int diff;
    Renderer* renderer = &ppu->renderer;
    uint16_t cycle = renderer->cycle;
    bool odd_cycle = (cycle & 1);
    if(renderer->sprite_count < 8) {
        if(odd_cycle) {
            renderer->sprite_y = ppu->oam.raw[(renderer->oam_n * 4) + renderer->oam_m];
        } else {
            if(renderer->oam_m == 0) {
                diff = (renderer->scanline + 1) - renderer->sprite_y;
                if(diff >= 0 && diff < (ppu->ppuctrl.SPRITESIZE == 1 ? 16 : 8)) {
                    renderer->secondary_oam->raw[renderer->sec_oam_index++] = renderer->sprite_y;
                    renderer->oam_m++;
                } else {
                    renderer->oam_n++;
                    if(renderer->oam_n >= 64) {
                        renderer->sprite_eval_done = true;
                    }
                }
            } else {
                renderer->secondary_oam->raw[renderer->sec_oam_index++] = renderer->sprite_y;
                renderer->oam_m++;
                if(renderer->oam_m == 4) {
                    renderer->oam_m = 0;
                    renderer->oam_n++;
                    renderer->sprite_count++;
                    if(renderer->oam_n >= 64) {
                        renderer->sprite_eval_done = true;
                    }
                }
            }
        }
    } else {
        if(odd_cycle) {
            renderer->sprite_y = ppu->oam.raw[(renderer->oam_n * 4) + renderer->oam_m];
        } else {
            diff = (renderer->scanline + 1) - renderer->sprite_y;
            if(diff >= 0 && diff < (ppu->ppuctrl.SPRITESIZE == 1 ? 16 : 8)) {
                renderer->sprite_overflow = true;
            }
            renderer->oam_m++;
            if(renderer->oam_m == 4) {
                renderer->oam_m = 0;
                renderer->oam_n++;
                if(renderer->oam_n >= 64) {
                    renderer->sprite_eval_done = true;
                }
            }
        }
    }
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
