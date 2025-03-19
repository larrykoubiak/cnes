#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include "input/input.h"

static uint8_t input_event_flags = 0;
static uint8_t prev_escape = 0, prev_p = 0, prev_s = 0, prev_d = 0;
// NES Controller state (8-bit register)
static uint8_t controller_state[2] = {0};
static uint8_t controller_shift[2] = {0};
static bool strobe = false;

// Map SDL keycodes to NES buttons
static const SDL_Scancode keymap[8] = {
    SDL_SCANCODE_X,   // A
    SDL_SCANCODE_W,   // B
    SDL_SCANCODE_RSHIFT, // Select
    SDL_SCANCODE_RETURN, // Start
    SDL_SCANCODE_UP,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_RIGHT
};

void input_poll() {
    SDL_PumpEvents();  // Update the keyboard state

    const bool *keystate = SDL_GetKeyboardState(NULL);
    
    controller_state[0] = 0; // Reset controller state before updating
    input_event_flags = 0;

    // Check just the keys you need
    if (!keystate[SDL_SCANCODE_ESCAPE] && prev_escape)
    input_event_flags |= INPUT_EVENT_QUIT;

    if (!keystate[SDL_SCANCODE_P] && prev_p)
    input_event_flags |= INPUT_EVENT_PAUSE;

    if (!keystate[SDL_SCANCODE_S] && prev_s)
    input_event_flags |= INPUT_EVENT_STEP;

    if (!keystate[SDL_SCANCODE_D] && prev_d)
    input_event_flags |= INPUT_EVENT_DUMP;

    prev_escape = keystate[SDL_SCANCODE_ESCAPE];
    prev_p = keystate[SDL_SCANCODE_P];
    prev_s = keystate[SDL_SCANCODE_S];
    prev_d = keystate[SDL_SCANCODE_D];
    for (int i = 0; i < 8; i++) {
        if (keystate[keymap[i]]) {
            controller_state[0] |= (1 << i);
        }
    }
}


// NES CPU writes to controller port (Strobe)
void controller_write(uint16_t addr, uint8_t value) {
    if (addr == 0x4016) {
        strobe = value & 1;
        if (strobe) {
            controller_shift[0] = controller_state[0];
            controller_shift[1] = controller_state[1];
        }
    }
}

// NES CPU reads from controller port (Shift Register)
uint8_t controller_read(uint16_t addr) {
    int index = (addr == 0x4016) ? 0 : 1;
    uint8_t bit = controller_shift[index] & 1;
    if (!strobe) {
        controller_shift[index] >>= 1;
    }
    return bit;
}

uint8_t input_get_events() {
    uint8_t events = input_event_flags;
    input_event_flags = 0; // Clear flags after reading
    return events;
}