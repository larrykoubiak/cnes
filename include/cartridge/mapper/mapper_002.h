#ifndef MAPPER_002_H
#define MAPPER_002_H

#include "mapper.h"

typedef struct {
    uint8_t prg_bank;  // Stores the currently selected PRG bank
} Mapper002State;

// Forward declarations
static void mapper002_init(Cartridge* cart);
static uint8_t mapper002_prg_read(Cartridge* cart, uint16_t address);
static void mapper002_prg_write(Cartridge* cart, uint16_t address, uint8_t value);
static uint8_t mapper002_chr_read(Cartridge* cart, uint16_t address);
static void mapper002_chr_write(Cartridge* cart, uint16_t address, uint8_t value);
static uint8_t mapper002_wram_read(Cartridge* cart, uint16_t address);
static void mapper002_wram_write(Cartridge* cart, uint16_t address, uint8_t value);
static void mapper002_free(Cartridge* cart);

extern Mapper MAPPER_002;

#endif // MAPPER_002_H
