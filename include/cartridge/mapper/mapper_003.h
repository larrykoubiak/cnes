#ifndef MAPPER_003_H
#define MAPPER_003_H

#include "mapper.h"

typedef struct {
    uint8_t prg_bank;  // Stores the currently selected PRG bank
} Mapper003State;

// Forward declarations
static void mapper003_init(Cartridge* cart);
static uint8_t mapper003_prg_read(Cartridge* cart, uint16_t address);
static void mapper003_prg_write(Cartridge* cart, uint16_t address, uint8_t value);
static uint8_t mapper003_chr_read(Cartridge* cart, uint16_t address);
static void mapper003_chr_write(Cartridge* cart, uint16_t address, uint8_t value);
static uint8_t mapper003_wram_read(Cartridge* cart, uint16_t address);
static void mapper003_wram_write(Cartridge* cart, uint16_t address, uint8_t value);
static void mapper003_free(Cartridge* cart);

extern Mapper MAPPER_003;

#endif // MAPPER_003_H
