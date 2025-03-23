#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
typedef struct {
    uint8_t controller_state[2];
    uint8_t controller_shift[2];
    bool strobe;
} Controller;

void controller_init(Controller* controller);
void controller_write(Controller* controller, uint16_t addr, uint8_t value);
uint8_t controller_read(Controller* controller, uint16_t addr);

#endif
