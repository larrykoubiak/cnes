#ifndef SDL_CONTEXT_H
#define SDL_CONTEXT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct sdl_context {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    SDL_Texture *texture;
} sdl_context;

int sdl_init(sdl_context *ctx, uint8_t *colors, int width, int height, const char *title);

void sdl_cleanup(sdl_context *ctx);

#endif
