#include <stdio.h>
#include "frontend/sdl_context.h"

void input_init(Input_Frontend* input) {
    // gamepad support
    int num_gamepads = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&num_gamepads);
    if(gamepads && num_gamepads > 0) {
        input->gamepad = SDL_OpenGamepad(gamepads[0]);
        if(!input->gamepad) {
            printf("Warning: Unable to open game controller: %s\n", SDL_GetError());
        } else {
            printf("Gamepad connected: %s\n", SDL_GetGamepadName(input->gamepad));
        }
    } else {
        printf("Gamepad found: %d\n", num_gamepads);
    }
    if(gamepads) {
        SDL_free(gamepads);
    }
}

uint8_t input_update(Input_Frontend* input, uint8_t* controller_state) {
    SDL_PumpEvents();
    const bool *keystate = SDL_GetKeyboardState(NULL);
    *controller_state = 0;
    input->input_event_flags = 0;

    if (!keystate[SDL_SCANCODE_ESCAPE] && input->prev_escape)
    input->input_event_flags |= EVENT_QUIT;
    if (!keystate[SDL_SCANCODE_P] && input->prev_p)
    input->input_event_flags |= EVENT_PAUSE;
    if (!keystate[SDL_SCANCODE_S] && input->prev_s)
    input->input_event_flags |= EVENT_STEP;
    if (!keystate[SDL_SCANCODE_D] && input->prev_d)
    input-> input_event_flags |= EVENT_DUMP;

    input->prev_escape = keystate[SDL_SCANCODE_ESCAPE];
    input->prev_p = keystate[SDL_SCANCODE_P];
    input->prev_s = keystate[SDL_SCANCODE_S];
    input->prev_d = keystate[SDL_SCANCODE_D];
    for (int i = 0; i < 8; i++) {
        if (keystate[keymap[i]]) {
            *controller_state |= (1 << i);
        }
    }
    if(input->gamepad) {
        for(int i=0;i < 8; i++) {
            if(SDL_GetGamepadButton(input->gamepad, gamepad_map[i])) {
                *controller_state |= (1 << i);
            }
        }
    }
}

uint8_t input_get_events(Input_Frontend* input) {
    uint8_t events = input->input_event_flags;
    input->input_event_flags = 0; // Clear flags after reading
    return events;
}

void input_cleanup(Input_Frontend* input) {
    if (input->gamepad) SDL_CloseGamepad(input->gamepad);
}