#include <stdio.h>
#include "frontend/sdl_context.h"

int video_init(Video_Frontend *video, int width, int height, const char *title) {
    video->window = SDL_CreateWindow(title, width, height, 0);
    if (!video->window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    video->renderer = SDL_CreateRenderer(video->window, NULL);
    if (!video->renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    video->ppu_surface = SDL_CreateSurface(256, 240, SDL_PIXELFORMAT_RGB24);
    if(!video->ppu_surface) {
        printf("SDL_CreateSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(video->renderer);
        SDL_DestroyWindow(video->window);
        SDL_Quit();
        return -1;
    }
    video->disasm_surface = SDL_CreateSurface(256, 240, SDL_PIXELFORMAT_RGB24);
    if(!video->disasm_surface) {
        printf("SDL_CreateSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(video->renderer);
        SDL_DestroyWindow(video->window);
        SDL_Quit();
        return -1;
    }
    video->emu_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB24);
    if(!video->emu_surface) {
        printf("SDL_CreateSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(video->renderer);
        SDL_DestroyWindow(video->window);
        SDL_Quit();
        return -1;
    }
    video->texture = SDL_CreateTexture(video->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!video->texture) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(video->renderer);
        SDL_DestroyWindow(video->window);
        SDL_Quit();
        return -1;
    }
    video->details = SDL_GetPixelFormatDetails(video->emu_surface->format);
    video->background_color = SDL_MapRGB(video->details,NULL,0,0,0);
    return 0;
}

void video_update(Video_Frontend *video, uint8_t *framebuffer) {
    // clear surfaces
    SDL_FillSurfaceRect(video->emu_surface, NULL, video->background_color);
    SDL_FillSurfaceRect(video->disasm_surface, NULL, video->background_color);
    // render ppu
    SDL_memcpy(video->ppu_surface->pixels, framebuffer, 256 * 240 * 3);
    SDL_Rect ppu_rect = {0,0,256,240};
    SDL_BlitSurface(video->ppu_surface, NULL, video->emu_surface, &ppu_rect);
    // render text
    SDL_Rect disasm_rect = {256,0,256,240};
    SDL_BlitSurface(video->disasm_surface, NULL, video->emu_surface, &disasm_rect);
    // render emu surface to texture
    SDL_UpdateTexture(video->texture, NULL, video->emu_surface->pixels, video->emu_surface->pitch);
    SDL_RenderTexture(video->renderer, video->texture, NULL, NULL);
    SDL_RenderPresent(video->renderer);
}

void video_cleanup(Video_Frontend *video) {
    if (video->texture) SDL_DestroyTexture(video->texture);
    if (video->emu_surface) SDL_DestroySurface(video->emu_surface);
    if (video->disasm_surface) SDL_DestroySurface(video->disasm_surface);
    if (video->ppu_surface) SDL_DestroySurface(video->ppu_surface);
    if (video->renderer) SDL_DestroyRenderer(video->renderer);
    if (video->window) SDL_DestroyWindow(video->window);
}