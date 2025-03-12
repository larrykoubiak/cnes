#ifndef APU_H
#define APU_H

#include <stdint.h>

typedef struct Timer {
    unsigned int timer: 11;
    unsigned int length_counter: 5;
} Timer;

typedef struct Envelope {
    unsigned int volume: 4;
    unsigned int constant_volume: 1;
    unsigned int loop_envelope: 1;
    unsigned int duty: 2;
} Envelope;

typedef struct Sweep {
    unsigned int shift: 3;
    unsigned int negate: 1;
    unsigned int period: 3;
    unsigned int enabled: 1;
} Sweep;

typedef struct LinearCounter {
    unsigned int linear_counter: 7;
    unsigned int length_halt: 1;
} LinearCounter;

typedef struct NoisePeriod {
    unsigned int period: 4;
    unsigned int unused: 3;
    unsigned int loop_noise: 1;
    unsigned int unused2: 3;
    unsigned int length_counter: 5;
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