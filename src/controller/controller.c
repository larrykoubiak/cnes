#include <stdint.h>
#include <stdbool.h>
#include "controller/controller.h"

#include <SDL3/SDL.h>

void controller_init(Controller* controller) {
    controller->controller_state[0] = 0;
    controller->controller_state[1] = 0;
    controller->controller_shift[0] = 0;
    controller->controller_shift[1] = 0;
}

void controller_write(Controller* controller, uint16_t addr, uint8_t value) {
    if (addr == 0x4016) {
        controller->strobe = value & 1;
        if (controller->strobe) {
            controller->controller_shift[0] = controller->controller_state[0];
            controller->controller_shift[1] = controller->controller_state[1];
        }
    }
}

uint8_t controller_read(Controller* controller, uint16_t addr) {
    int index = (addr == 0x4016) ? 0 : 1;
    uint8_t bit = controller->controller_shift[index] & 1;
    if (!controller->strobe) {
        controller->controller_shift[index] >>= 1;
    }
    return bit;
}
