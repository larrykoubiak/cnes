#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "rendering.h"

typedef struct Bus Bus;

typedef enum {
    HORIZONTAL = 0,
    VERTICAL = 1,
    FOUR_SCREEN = 8,
    SINGLE_SCREEN_LOWER = 9,
    SINGLE_SCREEN_UPPER = 10
} MirroringMode;

typedef enum {
    LOAD_COLORS_OK = 0,
    LOAD_COLORS_OK_FILE_ERROR = 1,
    LOAD_COLORS_OK_READ_ERROR = 2
} LoadColorsResult;

typedef union PPUCTRLRegister{
    struct {
        unsigned int NAMETABLE_X:1;
        unsigned int NAMETABLE_Y:1;
        unsigned int VRAM_DIRECTION:1;
        unsigned int SPRITETABLE:1;
        unsigned int BGTABLE:1;
        unsigned int SPRITESIZE:1;
        unsigned int PPU_EXT:1;
        unsigned int NMI_VBL:1;
    };
    uint8_t value;
} __attribute__((packed)) PPUCTRLRegister;

typedef union PPUMASKRegister{
    struct {
        unsigned int GREYSCALE:1;
        unsigned int SHOW_LEFT8_BG:1;
        unsigned int SHOW_LEFT8_SP:1;
        unsigned int BG_RENDER:1;
        unsigned int SP_RENDER:1;
        unsigned int HL_RED:1;
        unsigned int HL_GREEN:1;
        unsigned int HL_BLUE:1;
    };
    uint8_t value;
} __attribute__((packed)) PPUMASKRegister;

typedef union PPUSTATRegister {
    struct {
        unsigned int UNUSED:5;
        unsigned int SP_OVERFLOW:1;
        unsigned int SP0_HIT:1;
        unsigned int VBLANK:1;
    };
    uint8_t value;
} __attribute__((packed)) PPUSTATRegister;

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

typedef struct PPU {
    // Memory
    uint8_t vram[0x1000];
    union {
        uint8_t raw[0x100];
        Sprite sprites[0x40];
    }  oam;
    Sprite  secondary_oam[0x8];
    uint8_t palette[0x20];
    uint8_t colors[0x600];
    uint8_t pixel_buffer[0x100];
    uint8_t framebuffer[0xF000];
    uint8_t framebuffer_rgb[0x2D000];
    // Pointers
    Bus* bus;
    // Registers ($2000-$2007)
    PPUCTRLRegister ppuctrl;
    PPUMASKRegister ppumask;
    PPUSTATRegister ppustatus;
    uint8_t oamaddr;
    uint16_t ppuaddr;
    uint8_t ppudata;
    // Loopy registers
    LOOPYRegister v;
    LOOPYRegister t;
    uint8_t x;
    uint8_t w;
    // Internal state
    MirroringMode mirroring;
    uint64_t cycle;
    int scanline;
    bool frame_odd;
    uint8_t open_bus_val;
    uint64_t frame_count;
    uint8_t sprite_count;
    bool vblank_triggered;
} PPU;

int ppu_init(PPU* ppu, struct Bus* bus, MirroringMode mode);
void ppu_reset(PPU* ppu);
uint8_t ppu_register_read(PPU* ppu, uint16_t address);
void ppu_register_write(PPU* ppu, uint16_t address, uint8_t value);
uint8_t ppu_read(PPU* ppu, uint16_t address);
void ppu_write(PPU* ppu, uint16_t address, uint8_t value);


#endif