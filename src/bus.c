#include "bus.h"

int init_bus(Bus* bus, const char* filename) {
    bus->cycles = 0;
    if (cart_init(&bus->cart, filename) != 0) {
        fprintf(stderr, "Error during cartridge initialization");
        return 1;
    }
    cpu_init(&bus->cpu, bus);
    ppu_init(&bus->ppu, bus, cart_get_mirroring_mode(&bus->cart.header));
    memset(bus->ram,0, sizeof(bus->ram));
    return 0;
}

void free_bus(Bus* bus) {
    cart_free(&bus->cart);
}

uint8_t cpu_read(Bus* bus, uint16_t address) {
    if (address < 0x2000) {
        return bus->ram[address & 0x07FF];
    }
    if (address < 0x4000) {
        return ppu_register_read(&bus->ppu, address & 0x07);
    }
    if (address < 0x4015) {
        return 0; // Open bus from APU
    }
    if (address == 0x4015) {
        return apu_read(&bus->apu, address & 0x1F);
        return 0;
    }
    if (address < 0x4018) {
        return controller_read(address);
    }
    if (address < 0x6000) {
        return 0; //Unknown, possibly ROM
    }
    if (address < 0x8000) {
        return bus->cart.mapper->read_wram(&bus->cart, address & 0x1FFF); //Usually PRG-RAM if present
    }
    if (address < 0x10000) {
        return bus->cart.mapper->read_prg(&bus->cart, address & 0x7FFF);
    }
    return 0;
};

void cpu_write(Bus* bus, uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        bus->ram[address & 0x7FF] = value;
        return;
    }
    if (address < 0x4000) {
        ppu_register_write(&bus->ppu, address & 0x07, value);
        return;
    }
    if (address < 0x4014) {
        apu_write(&bus->apu, address & 0x1F, value);
        return;
    }
    if (address == 0x4014) {
        oam_dma(bus, value);
        return;
    }
    if (address == 0x4015) {
        apu_write(&bus->apu, address & 0x1F, value);
        return;
    }
    if (address == 0x4016) {
        controller_write(address, value); // Joystick strobe # TODO
    }
    if (address == 0x4017) {
        apu_write(&bus->apu, address & 0x1F, value);
        return;
    }
    if (address < 0x6000) {
        return; //Unknown, possibly ROM
    }
    if (address < 0x8000) {
        bus->cart.mapper->write_wram(&bus->cart, address & 0x1FFF, value); //Usually PRG-RAM if present
    }
    if (address < 0x10000) {
        bus->cart.mapper->write_prg(&bus->cart, address & 0x7FFF, value);
        return;
    }
    return;
};

void bus_step(Bus* bus) {
    int ppu_cycles;
    cpu_step(&bus->cpu);
    ppu_cycles = bus->cpu.cycles * 3;
    for (int i=0; i<ppu_cycles; i++) {
        ppu_step(&bus->ppu);
    }
    bus->cycles += bus->cpu.cycles;
}

void bus_trigger_nmi(Bus* bus) {
    bus->cpu.nmi_pending = 1;
}

void oam_dma(Bus* bus, uint8_t value) {
    uint16_t ram_address = value << 8;
    int i, p;
    for(p=0;p<6;p++) {
        ppu_step(&bus->ppu);
    }
    for (i=0; i<256; i++) {
        for(p=0;p<6;p++) {
            ppu_step(&bus->ppu);
        }
        bus->ppu.oam.raw[i] = bus->ram[ram_address + i];
    }
    bus->cycles += 514;
}

MirroringMode cart_get_mirroring_mode(const iNESHeader* header) {
    if (header->alternate_layout) {
        return FOUR_SCREEN;
    }
    switch(header->nametable_layout) {
        case HORIZONTAL_LAYOUT:
            return VERTICAL;
        case VERTICAL_LAYOUT:
            return HORIZONTAL;
        default:
            return HORIZONTAL;
    }
}