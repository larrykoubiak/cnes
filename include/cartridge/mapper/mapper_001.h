#ifndef MAPPER_001_H
#define MAPPER_001_H

#include "mapper.h"

// Forward declarations
static void mapper001_init(Cartridge* cart);
static uint8_t mapper001_prg_read(Cartridge* cart, uint16_t address);
static void mapper001_prg_write(Cartridge* cart, uint16_t address, uint8_t value);
static uint8_t mapper001_chr_read(Cartridge* cart, uint16_t address);
static void mapper001_chr_write(Cartridge* cart, uint16_t address, uint8_t value);
static uint8_t mapper001_wram_read(Cartridge* cart, uint16_t address);
static void mapper001_wram_write(Cartridge* cart, uint16_t address, uint8_t value);
static void mapper001_update_banks(Cartridge* cart);
static void mapper001_free(Cartridge* cart);

typedef struct {
    uint8_t shift_register;
    union {
        struct {
            unsigned int mirroring: 2;
            unsigned int prg_bank_mode: 2;
            unsigned int chr_bank_mode: 1;
            unsigned int unused: 3;
        };
        uint8_t value;
    } control;
    uint8_t chr_bank_0;
    uint8_t chr_bank_1;
    uint8_t prg_bank;
    uint8_t prg_ram_enable;
} Mapper001State;

extern Mapper MAPPER_001;

#endif // MAPPER_001_H
