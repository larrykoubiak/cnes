#ifndef RENDERING_H
#define RENDERING_H

#include <stdint.h>

typedef struct PPU PPU;

typedef union SPRITEATTR {
    struct {
        uint8_t palette:2;
        uint8_t unused:3;
        uint8_t priority:1;
        uint8_t flip_h:1;
        uint8_t flip_v:1;
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
        uint16_t coarse_x:5;
        uint16_t coarse_y:5;
        uint16_t nametable_x:1;
        uint16_t nametable_y:1;
        uint16_t fine_y:3;
        uint16_t unused:1;
    };
    uint16_t value;
} __attribute__((packed)) LOOPYRegister;

typedef struct Renderer {
    union {
        uint8_t raw[0x20];
        Sprite sprites[0x8];
    } secondary_oam;
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
    uint8_t sprite_count;
    bool sprite_overflow;
    bool sprite_zero_possible;
    bool sprite_zero_rendered;
    uint8_t sprite_y;
    uint8_t sprite_shifter_pattern_low[8];
    uint8_t sprite_shifter_pattern_high[8];
    SPRITEATTR sprite_attributes[8];
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
void fetch_sprite(PPU* ppu, int id);

#endif