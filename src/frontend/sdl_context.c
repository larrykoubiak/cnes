#include <stdio.h>
#include "frontend/sdl_context.h"

SDL_Color sdl_colors[64];

int sdl_init(sdl_context *ctx, uint8_t *colors, int width, int height, const char *title) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    ctx->window = SDL_CreateWindow(title, width, height, 0);
    if (!ctx->window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, NULL);
    if (!ctx->renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }

    for (int i=0; i< 64; i++) {
        sdl_colors[i].r = colors[i *3];
        sdl_colors[i].g = colors[(i *3) + 1];
        sdl_colors[i].b = colors[(i *3) + 2];
        sdl_colors[i].a = 255;
    }
    ctx->surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
    if(!ctx->surface) {
        printf("SDL_CreateSurface ERror: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }
    SDL_Palette *palette = SDL_CreatePalette(64);
    SDL_SetPaletteColors(palette, sdl_colors, 0, 64);
    SDL_SetSurfacePalette(ctx->surface, palette);

    ctx->texture = SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!ctx->texture) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }

    return 0;
}

void sdl_cleanup(sdl_context *ctx) {
    if (ctx->texture) SDL_DestroyTexture(ctx->texture);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window) SDL_DestroyWindow(ctx->window);
    SDL_Quit();
}
