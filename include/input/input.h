#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

#define INPUT_EVENT_QUIT  (1 << 0)
#define INPUT_EVENT_SAVE  (1 << 1)
#define INPUT_EVENT_PAUSE (1 << 2)

void input_poll();
void controller_write(uint16_t addr, uint8_t value);
uint8_t controller_read(uint16_t addr);
uint8_t input_get_events();

#endif
