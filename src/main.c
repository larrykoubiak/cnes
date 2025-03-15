#include "bus.h"
#include "frontend/sdl_context.h"

static char disassembly_cache[0x10000][16];

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
    memset(&disassembly_cache,0, sizeof(disassembly_cache));
    return 0;
}

void save_disassembly_cache() {
    FILE *file = fopen("log.txt", "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open log.txt for writing.\n");
        return;
    }
    for (uint16_t addr = 0; addr < 0xFFF0; addr++) {
        if (disassembly_cache[addr][0] != 0) {
            fprintf(file, "0x%04X %s\n", addr, disassembly_cache[addr]);
        }
    }
    fclose(file);
    printf("Disassembly cache saved to log.txt\n");
}

void store_disassembly(uint16_t address, const char* instruction) {
    strncpy(disassembly_cache[address], instruction, 15);
    disassembly_cache[address][15] = 0;
}

void step(Bus* bus) {
    uint16_t offset = bus->cpu.PC;
    bus_step(bus);
    store_disassembly(offset, bus->cpu.disassembly);
}

int main(int argc, char *argv[]) {
    Bus bus;
    sdl_context ctx;
    bool running = true;
    bool paused = false;
    void *pixels;
    int pitch;
    if (init(&bus, &ctx, argc, argv) != 0) {
        fprintf(stderr, "Initialization failed.\n");
        return 1;
    }
    cpu_reset(&bus.cpu);
    ppu_reset(&bus.ppu);
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }
        input_poll();
        uint8_t events = input_get_events();
        if (events & INPUT_EVENT_QUIT) {
            running = false;
        }
        if (events & INPUT_EVENT_PAUSE) {
            paused = !paused;
            if(paused) {
                printf("Paused\n");
            } else {
                printf("Resumed\n");
            }
        }
        if (events & INPUT_EVENT_SAVE) {
            save_disassembly_cache();
        }
        step(&bus);
        if(bus.ppu.vblank_triggered==1) {
            SDL_memcpy(ctx.surface->pixels, bus.ppu.framebuffer, 256 * 240);
            SDL_Surface *converted = SDL_ConvertSurface(ctx.surface, SDL_PIXELFORMAT_RGB24);
            if (converted) {
                SDL_UpdateTexture(ctx.texture, NULL, converted->pixels, converted->pitch);
                SDL_DestroySurface(converted);
            }
            SDL_RenderTexture(ctx.renderer, ctx.texture, NULL, NULL);
            SDL_RenderPresent(ctx.renderer);
            bus.ppu.vblank_triggered=0;
        }
    }
    free_bus(&bus);
    sdl_cleanup(&ctx);
    return 0;
}