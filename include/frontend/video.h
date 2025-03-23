#ifndef FRONTEND_VIDEO_H
#define FRONTEND_VIDEO_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *ppu_surface;
    SDL_Surface *disasm_surface;
    SDL_Surface *emu_surface;
    SDL_Texture *texture;
    const SDL_PixelFormatDetails *details;
    uint32_t background_color;
} Video_Frontend;

int video_init(Video_Frontend *video, int width, int height, const char *title);
void video_update(Video_Frontend *video, uint8_t *framebuffer);
void video_cleanup(Video_Frontend *video);
#endif