#ifndef FRONTEND_TEXT_H
#define FRONTEND_TEXT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    TTF_Font *font;
    SDL_Surface *disasm_surface;
} Text_Frontend;

int text_init(Text_Frontend *text, SDL_Surface *disasm_surface);
void text_update(Text_Frontend *text, char (*disasm)[16], uint16_t PC);
void text_cleanup(Text_Frontend *text);
#endif