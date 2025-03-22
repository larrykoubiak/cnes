#include "apu/dmc.h"

void dmc_write(DMCChannel *dmc, int reg_index, uint8_t value) {
    dmc->reg.raw[reg_index] = value;
    switch(reg_index) {
        case 0:
            break;
        case 1:
            dmc->dac = value & 0x7F;
            break;
        case 2:
            dmc->initial_address = 0xC000 + (value << 6);
            break;
        case 3:
            dmc->initial_length = (value << 4) + 1;
            break;
    }
}

void dmc_step(DMCChannel *dmc) {
    if (dmc->timer_counter == 0) {
        dmc->timer_counter = dmc->timer_period;
    } else {
        dmc->timer_counter--;
    }
}

void dmc_read(DMCChannel *dmc, uint8_t (*read_byte_function)(uint16_t addr)) {
    // TO DO
}

int dmc_get_output_amplitude(const DMCChannel *dmc) {
    // TO DO
}