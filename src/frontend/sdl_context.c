#include <stdio.h>
#include "frontend/sdl_context.h"

int sdl_init(sdl_context *ctx, uint8_t *colors, int width, int height, const char *title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }
    audio_init(&ctx->audio);
    input_init(&ctx->input);
    video_init(&ctx->video, width, height, title);
    text_init(&ctx->text, ctx->video.disasm_surface);
    return 0;
}

uint8_t sdl_update(sdl_context *ctx, uint8_t *controller_state) {
    uint8_t events = 0;
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_EVENT_QUIT) {
            events |= EVENT_QUIT;
        }
    }
    input_update(&ctx->input, controller_state);
    events |= input_get_events(&ctx->input);
    return events;
}

void sdl_render(sdl_context* ctx, uint8_t* framebuffer, char (*disasm)[16], uint16_t PC, bool paused) {
    if(paused) {
        text_update(&ctx->text, disasm, PC);
    }
    video_update(&ctx->video, framebuffer);
}

void sdl_cleanup(sdl_context* ctx) {
    video_cleanup(&ctx->video);
    text_cleanup(&ctx->text);
    input_cleanup(&ctx->input);
    audio_cleanup(&ctx->audio);
    SDL_Quit();
}
