#ifndef APU_BITFIELDS_H
#define APU_BITFIELDS_H

#include <stdint.h>

// Register bitfields are intentionally decoded with masks in the channel
// implementations. C bitfield order and packing are implementation-defined.
static inline uint8_t apu_length_counter_value(uint8_t index) {
    static const uint8_t length_table[32] = {
        10, 254, 20,  2, 40,  4, 80,  6,
        160, 8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22,
        192, 24, 72, 26, 16, 28, 32, 30
    };

    return length_table[index & 0x1FU];
}

#endif
