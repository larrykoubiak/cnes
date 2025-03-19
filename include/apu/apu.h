#ifndef APU_H
#define APU_H

#include <stdint.h>

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

typedef union PulseChannel {
    unsigned char raw[4];
    struct {
        Envelope envelope;
        Sweep sweep;
        Timer timer;
    };
} PulseChannel;

typedef union TriangleChannel {
    unsigned char raw[4];
    struct {
        LinearCounter counter;
        unsigned char unused;
        Timer timer;
    };
} TriangleChannel;

typedef union NoiseChannel {
    unsigned char raw[4];
    struct {
        Envelope envelope;
        unsigned char unused;
        NoisePeriod period;
    };
} NoiseChannel;

typedef union DMCChannel {
    unsigned char raw[4];
    struct {
        DMCControl control;
        unsigned char counter;
        unsigned char address;
        unsigned char length;
    };
} DMCChannel;

typedef struct APU {
    PulseChannel pulse1;
    PulseChannel pulse2;
    TriangleChannel triangle;
    NoiseChannel noise;
    DMCChannel dmc;
    uint8_t status;
    uint8_t frame_counter;
} APU;

void apu_reset(APU* apu);
void apu_write(APU* apu, uint16_t addr, uint8_t value);
uint8_t apu_read(APU* apu, uint16_t addr);

#endif