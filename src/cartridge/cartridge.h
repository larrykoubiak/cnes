#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ines.h"
#include "mapper.h"

typedef struct Cartridge {
    iNESHeader header;
    uint8_t* prg_rom;
    uint32_t prg_rom_size;
    uint8_t* chr_rom;
    uint32_t chr_rom_size;
    uint8_t* prg_ram;
    uint32_t prg_ram_size;
    uint8_t* chr_ram;
    uint32_t chr_ram_size;
    Mapper* mapper;
} Cartridge;

typedef enum {
    LOAD_CARTRIDGE_OK = 0,
    LOAD_CARTRIDGE_FILE_ERROR = 1,
    LOAD_CARTRIDGE_HEADER_ERROR = 2,
    LOAD_CARTRIDGE_MEM_ERROR = 3,
    LOAD_CARTRIDGE_READ_ERROR = 4
} LoadCartridgeResult;

int cart_init(Cartridge* cart, const char* filename);
LoadCartridgeResult cart_load(Cartridge* cart, const char* filename);
void cart_free(Cartridge* cart);

#endif