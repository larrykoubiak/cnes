#ifndef RENDERING_H
#define RENDERING_H

#include <stdint.h>

typedef struct PPU PPU;

typedef union SPRITEATTR {
    struct {
        unsigned int palette:2;
        unsigned int unused:3;
        unsigned int priority:1;
        unsigned int flip_h:1;
        unsigned int flip_v:1;
    };
    uint8_t value;
} __attribute__((packed)) SPRITEATTR;

typedef struct Sprite {
    uint8_t y;
    uint8_t tile_id;
    SPRITEATTR attr;
    uint8_t x;
} Sprite;

typedef union LOOPYRegister {
    struct {
        unsigned int coarse_x:5;
        unsigned int coarse_y:5;
        unsigned int nametable_x:1;
        unsigned int nametable_y:1;
        unsigned int fine_y:3;
        unsigned int unused:1;
    };
    uint16_t value;
} __attribute__((packed)) LOOPYRegister;

typedef struct Renderer {
    union {
        uint8_t raw[0x20];
        Sprite sprites[0x8];
    } secondary_oam[0x8];
    uint8_t pixel_buffer[0x100];
    uint8_t framebuffer[0xF000];
    uint8_t framebuffer_rgb[0x2D000];
    // Loopy registers
    LOOPYRegister v;
    LOOPYRegister t;
    uint8_t x;
    uint8_t w;
    // temp BG variables
    uint8_t bg_next_tile_id;
    uint8_t bg_next_attr;
    uint8_t bg_next_pattern_lsb;
    uint8_t bg_next_pattern_msb;
    uint16_t bg_shifter_pattern_low;
    uint16_t bg_shifter_pattern_high;
    uint16_t bg_shifter_attribute_low;
    uint16_t bg_shifter_attribute_high;
    // temp FG variables
    uint8_t oam_m;
    uint8_t oam_n;
    uint8_t sec_oam_index;
    uint8_t sprite_count;
    bool sprite_eval_done;
    bool sprite_overflow;
    uint8_t sprite_y;
    uint8_t sprite_shifter_pattern_low[8];
    uint8_t sprite_shifter_pattern_high[8];
    uint8_t sprite_x_counters[8];
    // Internal state
    uint16_t cycle;
    int scanline;
    bool frame_odd;
    uint64_t frame_count;
} Renderer;

void renderer_init(PPU* ppu);
void renderer_step(PPU* ppu);
void render_visible_scanline(PPU* ppu);
void render_prerender_scanline(PPU* ppu);
void render_pixel(PPU* ppu);
void render_rgb(PPU* ppu);

void load_shifters(PPU* ppu);
void update_shifters(PPU* ppu);
void read_NT(PPU* ppu);
void read_AT(PPU* ppu);
void read_BG_lsb(PPU* ppu);
void read_BG_msb(PPU* ppu);
void inc_hori(PPU* ppu);
void inc_vert(PPU* ppu);
void reset_hori(PPU* ppu);
void reset_vert(PPU* ppu);
void eval_sprite(PPU* ppu);

#endif