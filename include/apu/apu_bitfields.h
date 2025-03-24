#ifndef APU_BITFIELDS_H
#define APU_BITFIELDS_H

#include <stdint.h>
#include <stdbool.h>
#include "utils/constants.h"

static uint8_t length_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
    12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

typedef struct Timer {
    uint16_t timer: 11;
    uint8_t length_counter: 5;
} Timer;

typedef struct Envelope {
    uint8_t volume: 4;
    uint8_t constant_volume: 1;
    uint8_t loop_envelope: 1;
    uint8_t duty: 2;
} Envelope;

typedef struct Sweep {
    uint8_t shift: 3;
    uint8_t negate: 1;
    uint8_t period: 3;
    uint8_t enabled: 1;
} Sweep;

typedef struct LinearCounter {
    uint8_t linear_counter: 7;
    uint8_t length_halt: 1;
} LinearCounter;

typedef struct NoisePeriod {
    uint8_t period: 4;
    uint8_t unused: 3;
    uint8_t loop_noise: 1;
    uint8_t unused2: 3;
    uint8_t length_counter: 5;
} NoisePeriod;

typedef struct DMCControl {
    uint8_t rate_index: 4;
    uint8_t unused: 2;
    uint8_t loop_sample: 1;
    uint8_t irq_enable: 1;
} DMCControl;

#endif