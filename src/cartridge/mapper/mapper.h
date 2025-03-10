#ifndef MAPPER_H
#define MAPPER_H

#include <stdint.h>

typedef struct Cartridge Cartridge;

typedef void (*InitFunc)(Cartridge* cart);
typedef uint8_t (*ReadFunc)(Cartridge* cart, uint16_t address);
typedef void (*WriteFunc)(Cartridge* cart, uint16_t address, uint8_t value);
typedef void (*FreeFunc)(Cartridge* cart);

typedef struct Mapper {
    InitFunc init;
    ReadFunc read_prg;
    WriteFunc write_prg;
    ReadFunc read_chr;
    WriteFunc write_chr;
    FreeFunc free;
    union {
        void* state;
        uint8_t storage[8];
    };
} Mapper;

Mapper* get_mapper(uint16_t mapper_id);

#endif