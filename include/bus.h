#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include "apu/apu.h"
#include "cartridge/cartridge.h"
#include "cpu/cpu.h"
#include "controller/controller.h"
#include "ppu/ppu.h"

typedef struct Bus {
    uint8_t ram[0x800];  //2KB RAM
    APU apu;
    Cartridge cart;
    Controller controller;
    CPU cpu;
    PPU ppu;
    uint64_t cycles;
    uint8_t dma_page;
    uint8_t dma_address;
    uint8_t dma_data;
    bool dma_dummy;
    bool dma_transfer;
} Bus;

int init_bus(Bus* bus, const char* filename);
void free_bus(Bus* bus);
uint8_t cpu_read(Bus* bus, uint16_t address);
void cpu_write(Bus* bus, uint16_t address, uint8_t value);
void bus_step(Bus* bus);
void bus_trigger_nmi(Bus* bus);

#endif