#include <stdio.h>
#include "frontend/sdl_context.h"

int sdl_init(sdl_context *ctx, uint8_t *colors, int width, int height, const char *title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }
    if (!TTF_Init()) {
        printf("TTF_Init Error: %s\n", SDL_GetError());
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
    ctx->ppu_surface = SDL_CreateSurface(256, 240, SDL_PIXELFORMAT_RGB24);
    if(!ctx->ppu_surface) {
        printf("SDL_CreateSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }
    ctx->disasm_surface = SDL_CreateSurface(256, 240, SDL_PIXELFORMAT_RGB24);
    if(!ctx->disasm_surface) {
        printf("SDL_CreateSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }
    ctx->emu_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB24);
    if(!ctx->emu_surface) {
        printf("SDL_CreateSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }
    ctx->texture = SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!ctx->texture) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }
    ctx->font = TTF_OpenFont("arcade-legacy.ttf", 10.0f);
    if (!ctx->font) {
        printf("Failed to load font: %s\n", SDL_GetError());
        return 1;
    }
    ctx->details = SDL_GetPixelFormatDetails(ctx->emu_surface->format);
    // gamepad support
    int num_gamepads = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&num_gamepads);
    if(gamepads && num_gamepads > 0) {
        ctx->gamepad = SDL_OpenGamepad(gamepads[0]);
        if(!ctx->gamepad) {
            printf("Warning: Unable to open game controller: %s\n", SDL_GetError());
        } else {
            printf("Gamepad connected: %s\n", SDL_GetGamepadName(ctx->gamepad));
        }
    } else {
        printf("Gamepad found: %d\n", num_gamepads);
    }
    if(gamepads) {
        SDL_free(gamepads);
    }
    return 0;
}

void sdl_render(sdl_context *ctx, uint8_t *framebuffer, char (*disasm)[16], uint16_t PC, bool paused) {
    uint32_t color = SDL_MapRGB(ctx->details,NULL,0,0,0);
    SDL_FillSurfaceRect(ctx->emu_surface, NULL, color);
    // render ppu
    SDL_memcpy(ctx->ppu_surface->pixels, framebuffer, 256 * 240 * 3);
    SDL_Rect ppu_rect = {0,0,256,240};
    SDL_BlitSurface(ctx->ppu_surface, NULL, ctx->emu_surface, &ppu_rect);
    // render text
    SDL_FillSurfaceRect(ctx->disasm_surface, NULL, color);
    if(paused) {
        SDL_Color white = {255,255,255,255};
        char text[32];
        uint16_t offset = PC;
        for(int y=0; y< 16; y++) {
            snprintf(text, 32, "%s 0x%04X %s", offset == PC ? "*":" ", offset, disasm[offset]);
            SDL_Surface *text_surface = TTF_RenderText_Solid(ctx->font, text, 0, white);
            SDL_Rect text_rect = {0,(y*12),text_surface->w,text_surface->h};
            SDL_BlitSurface(text_surface, NULL, ctx->disasm_surface, &text_rect);
            SDL_DestroySurface(text_surface);
            offset += 1;
            while(disasm[offset][0]==0) {
                offset += 1;
            }
        }
    }
    SDL_Rect disasm_rect = {256,0,256,240};
    SDL_BlitSurface(ctx->disasm_surface, NULL, ctx->emu_surface, &disasm_rect);
    // render emu surface to texture
    SDL_UpdateTexture(ctx->texture, NULL, ctx->emu_surface->pixels, ctx->emu_surface->pitch);
    SDL_RenderTexture(ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer);
}

void sdl_cleanup(sdl_context *ctx) {
    if (ctx->font) TTF_CloseFont(ctx->font);
    if (ctx->texture) SDL_DestroyTexture(ctx->texture);
    if (ctx->emu_surface) SDL_DestroySurface(ctx->emu_surface);
    if (ctx->disasm_surface) SDL_DestroySurface(ctx->disasm_surface);
    if (ctx->ppu_surface) SDL_DestroySurface(ctx->ppu_surface);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window) SDL_DestroyWindow(ctx->window);
    if (ctx->gamepad) SDL_CloseGamepad(ctx->gamepad);
    SDL_Quit();
}
