#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "ppu/rendering.h"

typedef struct Bus Bus;

typedef enum {
    LOAD_COLORS_OK = 0,
    LOAD_COLORS_OK_FILE_ERROR = 1,
    LOAD_COLORS_OK_READ_ERROR = 2
} LoadColorsResult;

typedef union PPUCTRLRegister{
    struct {
        uint8_t NAMETABLE_X:1;
        uint8_t NAMETABLE_Y:1;
        uint8_t VRAM_DIRECTION:1;
        uint8_t SPRITETABLE:1;
        uint8_t BGTABLE:1;
        uint8_t SPRITESIZE:1;
        uint8_t PPU_EXT:1;
        uint8_t NMI_VBL:1;
    };
    uint8_t value;
} __attribute__((packed)) PPUCTRLRegister;

typedef union PPUMASKRegister{
    struct {
        uint8_t GREYSCALE:1;
        uint8_t SHOW_LEFT8_BG:1;
        uint8_t SHOW_LEFT8_SP:1;
        uint8_t BG_RENDER:1;
        uint8_t SP_RENDER:1;
        uint8_t HL_RED:1;
        uint8_t HL_GREEN:1;
        uint8_t HL_BLUE:1;
    };
    uint8_t value;
} __attribute__((packed)) PPUMASKRegister;

typedef union PPUSTATRegister {
    struct {
        uint8_t UNUSED:5;
        uint8_t SP_OVERFLOW:1;
        uint8_t SP0_HIT:1;
        uint8_t VBLANK:1;
    };
    uint8_t value;
} __attribute__((packed)) PPUSTATRegister;

typedef struct PPU {
    // Memory
    uint8_t vram[0x1000];
    union {
        uint8_t raw[0x100];
        Sprite sprites[0x40];
    }  oam;
    uint8_t palette[0x20];
    uint8_t colors[0x600];
    // Pointers
    Bus* bus;
    // Registers ($2000-$2007)
    PPUCTRLRegister ppuctrl;
    PPUMASKRegister ppumask;
    PPUSTATRegister ppustatus;
    uint8_t oamaddr;
    uint16_t ppuaddr;
    uint8_t ppudata;
    uint8_t open_bus_val;
    MirroringMode mirroring;
    Renderer renderer;
    bool vblank_triggered;
} PPU;

void ppu_step(PPU* ppu);
int ppu_init(PPU* ppu, struct Bus* bus, MirroringMode mode);
void ppu_reset(PPU* ppu);
uint8_t ppu_register_read(PPU* ppu, uint16_t address);
void ppu_register_write(PPU* ppu, uint16_t address, uint8_t value);
uint8_t ppu_read(PPU* ppu, uint16_t address);
void ppu_write(PPU* ppu, uint16_t address, uint8_t value);
void ppu_trigger_nmi(PPU* ppu);

#endif