#include "bus.h"
#include "frontend/sdl_context.h"

int init(Bus* bus, sdl_context* ctx, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <romfile>\n", argv[0]);
        return 1;
    }
    int result_bus = init_bus(bus, argv[1]);
    if (result_bus != 0) {
        return result_bus;
    }
    int result_sdl = sdl_init(ctx, bus->ppu.colors, 256, 240, "NES Emulator");
    if (result_sdl != 0) {
        return result_sdl;
    }
    return 0;
}

void step(Bus* bus) {
    uint16_t offset = bus->cpu.PC;
    bus_step(bus);
}

int main(int argc, char *argv[]) {
    Bus bus;
    sdl_context ctx;
    int running;
    init(&bus, &ctx, argc, argv);
    cpu_reset(&bus.cpu);
    ppu_reset(&bus.ppu);
    SDL_Event event;
    running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }
        step(&bus);
        if(bus.ppu.vblank_triggered==1) {
            SDL_memcpy(ctx.surface->pixels, bus.ppu.framebuffer, 256 * 240);
            SDL_Surface *converted = SDL_ConvertSurface(ctx.surface, SDL_PIXELFORMAT_RGB24);
            SDL_UpdateTexture(ctx.texture, NULL, converted->pixels, converted->pitch);
            SDL_DestroySurface(converted);
            SDL_RenderTexture(ctx.renderer, ctx.texture, NULL, NULL);
            SDL_RenderPresent(ctx.renderer);
            bus.ppu.vblank_triggered=0;
        }
    }
    free_bus(&bus);
    sdl_cleanup(&ctx);
    return 0;
}