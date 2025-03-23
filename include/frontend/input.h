#ifndef FRONTEND_INPUT_H
#define FRONTEND_INPUT_H

#include <SDL3/SDL.h>

// Map SDL keycodes to NES buttons
static const SDL_Scancode keymap[8] = {
    SDL_SCANCODE_X,   // A
    SDL_SCANCODE_Z,   // B
    SDL_SCANCODE_RSHIFT, // Select
    SDL_SCANCODE_RETURN, // Start
    SDL_SCANCODE_UP,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_RIGHT
};
// Map SDL gamepad buttons to NES buttons
static const SDL_GamepadButton gamepad_map[8] = {
    SDL_GAMEPAD_BUTTON_EAST,      // A
    SDL_GAMEPAD_BUTTON_SOUTH,      // B
    SDL_GAMEPAD_BUTTON_BACK,   // Select
    SDL_GAMEPAD_BUTTON_START,  // Start
    SDL_GAMEPAD_BUTTON_DPAD_UP,
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT
};

typedef struct {
    SDL_Gamepad *gamepad;
    uint8_t input_event_flags;
    uint8_t prev_escape;
    uint8_t prev_p;
    uint8_t prev_s;
    uint8_t prev_d;
} Input_Frontend;

void input_init(Input_Frontend* input);
uint8_t input_update(Input_Frontend* input, uint8_t* controller_state);
void input_cleanup(Input_Frontend* input);

uint8_t input_get_events(Input_Frontend* input);
#endif