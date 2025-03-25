#ifndef SDL_CONTEXT_H
#define SDL_CONTEXT_H

#include <SDL3/SDL.h>
#include "frontend/audio.h"
#include "frontend/input.h"
#include "frontend/text.h"
#include "frontend/video.h"
#include "utils/constants.h"
#include "utils/ring_buffer.h"

typedef struct {
    Audio_Frontend audio;
    Input_Frontend input;
    Text_Frontend text;
    Video_Frontend video;
    uint32_t background_color;
} sdl_context;

int sdl_init(sdl_context *ctx, uint8_t *colors, int width, int height, const char *title);
uint8_t sdl_update(sdl_context *ctx, uint8_t *controller_state);
void sdl_render(sdl_context* ctx, bool paused, uint8_t* framebuffer, char (*disasm)[16], uint16_t PC, int fps);
void sdl_cleanup(sdl_context *ctx);

#endif
