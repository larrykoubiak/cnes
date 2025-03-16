#ifndef SDL_CONTEXT_H
#define SDL_CONTEXT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct sdl_context {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *ppu_surface;
    SDL_Surface *disasm_surface;
    SDL_Surface *emu_surface;
    SDL_Texture *texture;
    const SDL_PixelFormatDetails *details;
    TTF_Font *font;
} sdl_context;

int sdl_init(sdl_context *ctx, uint8_t *colors, int width, int height, const char *title);
void sdl_render(sdl_context *ctx, uint8_t *framebuffer, char (*disasm)[16], uint16_t PC, bool paused);
void sdl_cleanup(sdl_context *ctx);

#endif
