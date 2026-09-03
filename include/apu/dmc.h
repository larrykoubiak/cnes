#ifndef DMC_CHANNEL_H
#define DMC_CHANNEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t reg[4];
    bool     enabled;
    bool     irq_pending;

    uint16_t timer_counter;
    uint16_t timer_period;

    uint8_t  dac;

    uint8_t  shift_register;
    uint8_t  bits_remaining;

    uint16_t current_address; 
    uint16_t current_length;

    uint16_t initial_address; 
    uint16_t initial_length;

    bool     sample_buffer_has_data;
    uint8_t  sample_buffer;
} DMCChannel;

void dmc_write(DMCChannel *dmc, int reg_index, uint8_t value);

void dmc_step(DMCChannel *dmc);

void dmc_read(DMCChannel *dmc, uint8_t (*read_byte_function)(uint16_t addr));

uint8_t dmc_get_output_amplitude(const DMCChannel *dmc);

#endif
