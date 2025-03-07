#ifndef Bus_H
#define Bus_H

#include <stdint.h>
#include "cpu.h"
#include "cartridge.h"
#include "mapper.h"
#include "ppu.h"

typedef struct Bus {
    uint8_t ram[0x800];  //2KB RAM
    Cartridge cart;
    CPU cpu;
    PPU ppu;
    uint64_t cycles;
} Bus;
int init_bus(Bus* bus, const char* filename);
void free_bus(Bus* bus);
uint8_t cpu_read(Bus* bus, uint16_t address);
void cpu_write(Bus* bus, uint16_t address, uint8_t value);
void bus_step(Bus* bus);
void bus_trigger_nmi(Bus* bus);
void oam_dma(Bus* bus, uint8_t value);

#endif