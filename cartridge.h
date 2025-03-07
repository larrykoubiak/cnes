#ifndef Cartridge_H
#define Cartridge_H
#include <stdint.h>
#include "ines.h"
#include "mapper.h"

typedef enum {
    LOAD_CARTRIDGE_OK = 0,
    LOAD_CARTRIDGE_FILE_ERROR = 1,
    LOAD_CARTRIDGE_HEADER_ERROR = 2,
    LOAD_CARTRIDGE_MEM_ERROR = 3,
    LOAD_CARTRIDGE_READ_ERROR = 4
} LoadCartridgeResult;

typedef enum {
    HORIZONTAL = 0,
    VERTICAL = 1,
    FOUR_SCREEN = 8,
    SINGLE_SCREEN_LOWER = 9,
    SINGLE_SCREEN_UPPER = 10
} MirroringMode;

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
    struct Mapper* mapper;
} Cartridge;

int cart_init(Cartridge* cart, const char* filename);
LoadCartridgeResult cart_load(Cartridge* cart, const char* filename);
void cart_free(Cartridge* cart);
MirroringMode cart_get_mirroring_mode(const iNESHeader* header);

#endif