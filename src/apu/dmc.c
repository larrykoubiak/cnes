#include "apu/dmc.h"

static const uint16_t dmc_period_table[16] = {
    428, 380, 340, 320, 286, 254, 226, 214,
    190, 160, 142, 128, 106,  84,  72,  54
};

void dmc_write(DMCChannel *dmc, int reg_index, uint8_t value) {
    if (reg_index < 0 || reg_index > 3) {
        return;
    }

    dmc->reg[reg_index] = value;
    switch(reg_index) {
        case 0:
            dmc->timer_period = (uint16_t)(dmc_period_table[value & 0x0FU] - 1U);
            if ((value & 0x80U) == 0) {
                dmc->irq_pending = false;
            }
            break;
        case 1:
            dmc->dac = value & 0x7F;
            break;
        case 2:
            dmc->initial_address = (uint16_t)(0xC000U + ((uint32_t)value << 6));
            break;
        case 3:
            dmc->initial_length = (uint16_t)(((uint32_t)value << 4) + 1U);
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
    (void)dmc;
    (void)read_byte_function;
}

uint8_t dmc_get_output_amplitude(const DMCChannel *dmc) {
    // $4011 directly controls the DAC even when sample playback is disabled.
    return dmc->dac;
}
