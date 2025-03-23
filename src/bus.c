#include "bus.h"

int init_bus(Bus* bus, const char* filename) {
    bus->cycles = 0;
    bus->dma_page = 0x00;
    bus->dma_address = 0x00;
    bus->dma_data = 0x00;
    bus->dma_dummy = true;
    bus->dma_transfer = false;
    if (cart_init(&bus->cart, filename) != 0) {
        fprintf(stderr, "Error during cartridge initialization");
        return 1;
    }
    controller_init(&bus->controller);
    cpu_init(&bus->cpu, bus);
    ppu_init(&bus->ppu, bus, bus->cart.mapper->get_mirroring(&bus->cart));
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
        return controller_read(&bus->controller, address);
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
        bus->dma_page = value;
        bus->dma_address = 0x00;
        bus->dma_transfer = true;
        return;
    }
    if (address == 0x4015) {
        apu_write(&bus->apu, address & 0x1F, value);
        return;
    }
    if (address == 0x4016) {
        controller_write(&bus->controller, address, value); // Joystick strobe # TODO
        return;
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
        return;
    }
    if (address < 0x10000) {
        bus->cart.mapper->write_prg(&bus->cart, address & 0x7FFF, value);
        MirroringMode mm = bus->cart.mapper->get_mirroring(&bus->cart);
        if(mm != bus->ppu.mirroring) {
            bus->ppu.mirroring = mm;
        }
        return;
    }
    return;
};

void bus_step(Bus* bus) {
    ppu_step(&bus->ppu);
    if((bus->cycles % 3) == 0) {
        apu_step(&bus->apu);
        if(bus->dma_transfer) {
            if(bus->dma_dummy) {
                if(bus->cycles % 2 == 1) {
                    bus->dma_dummy = false;
                }
            } else {
                if(bus->cycles % 2 == 0) {
                    bus->dma_data = cpu_read(bus, (bus->dma_page << 8) | bus->dma_address);
                } else {
                    bus->ppu.oam.raw[bus->dma_address] = bus->dma_data;
                    bus->dma_address++;
                    if(bus->dma_address == 0x00) {
                        bus->dma_transfer = false;
                        bus->dma_dummy = true;
                    }
                }
            }
        } else {
            cpu_step(&bus->cpu);
        }
    }
    bus->cycles++;
}

void bus_trigger_nmi(Bus* bus) {
    bus->cpu.nmi_pending = 1;
}
