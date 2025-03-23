#include <stdio.h>
#include "frontend/sdl_context.h"

int text_init(Text_Frontend *text, SDL_Surface *disasm_surface) {
    if (!TTF_Init()) {
        printf("TTF_Init Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    text->font = TTF_OpenFont("arcade-legacy.ttf", 10.0f);
    if (!text->font) {
        printf("Failed to load font: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    text->disasm_surface = disasm_surface;
}

void text_clear(Text_Frontend *text, uint32_t color) {
    SDL_FillSurfaceRect(text->disasm_surface, NULL, color);
}

void text_update(Text_Frontend *text, char (*disasm)[16], uint16_t PC) {
    // render text
    SDL_Color white = {255,255,255,255};
    char buffer[32];
    uint16_t offset = PC;
    for(int y=0; y< 16; y++) {
        snprintf(buffer, 32, "%s 0x%04X %s", offset == PC ? "*":" ", offset, disasm[offset]);
        SDL_Surface *text_surface = TTF_RenderText_Solid(text->font, buffer, 0, white);
        SDL_Rect text_rect = {0,(y*12),text_surface->w,text_surface->h};
        SDL_BlitSurface(text_surface, NULL, text->disasm_surface, &text_rect);
        SDL_DestroySurface(text_surface);
        offset += 1;
        while(disasm[offset][0]==0) {
            offset += 1;
        }
    }
}

void text_cleanup(Text_Frontend *text) {
    if (text->font) TTF_CloseFont(text->font);
}