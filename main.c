#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bus.h"

void step(Bus* bus) {
    uint16_t offset = bus->cpu.PC;
    bus_step(bus);
    printf("0x%04X %s\n",offset, &bus->cpu.disassembly);
}

int main(int argc, char *argv[]) {
    Bus bus;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <romfile>\n", argv[0]);
        return 1;
    }
    int result = init_bus(&bus, argv[1]);
    if (result != 0) {
        return result;
    }
    printf("%s: prg_rom_size: %d chr_rom_size: %d\n",argv[1], bus.cart.prg_rom_size, bus.cart.chr_rom_size);
    cpu_reset(&bus.cpu);
    for(int i=0; i<10000; i++) {
        step(&bus);
    }
    free_bus(&bus);
    return 0;
}