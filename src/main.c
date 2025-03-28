#include "main.h"

int init(Bus* bus, sdl_context* ctx, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <romfile>\n", argv[0]);
        return 1;
    }
    int result_bus = init_bus(bus, argv[1],44100);
    if (result_bus != 0) {
        return result_bus;
    }
    int result_sdl = sdl_init(ctx, bus->ppu.colors, 1536, 720, "NES Emulator");
    if (result_sdl != 0) {
        return result_sdl;
    }
    memset(&disassembly_cache,0, sizeof(disassembly_cache));
    cpu_reset(&bus->cpu);
    ppu_reset(&bus->ppu);
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

void sync_frame(uint64_t* last_time_ns) {
    uint64_t now = SDL_GetTicksNS();
    if (*last_time_ns == 0) {
        *last_time_ns = now;
        return;
    }
    uint64_t elapsed = now - *last_time_ns;
    if (elapsed < FRAME_TIME_NS) {
        SDL_DelayNS((uint64_t)(FRAME_TIME_NS - elapsed));
        now = SDL_GetTicksNS(); // re-fetch after delay
    }
    *last_time_ns = now;
}

int main(int argc, char *argv[]) {
    Bus bus;
    sdl_context ctx;
    bool running = true;
    bool paused = false;
    uint8_t events = 0;
    int audio_cycle_counter = 0;
    uint64_t last_time_ns, fps_last_time;
    int fps_frame_count = 0;
    int current_fps = 0;
    last_time_ns = fps_last_time = SDL_GetTicksNS();
    
    if (init(&bus, &ctx, argc, argv) != 0) {
        fprintf(stderr, "Initialization failed.\n");
        return 1;
    }
    while (running) {
        // Loop for 1 frame or until paused
        while (!bus.ppu.vblank_triggered && !paused) {
            step(&bus);
            audio_cycle_counter++;
            if (audio_cycle_counter >= 89342) {
                audio_update(&ctx.audio, &bus.apu.sample_buffer, paused);
                audio_cycle_counter = 0;
            }
        }
        // Process events
        events = sdl_update(&ctx, &bus.controller.controller_state[0]);
        if (events & EVENT_QUIT) {
            running = false;
        }
        if (events & EVENT_PAUSE) {
            paused = !paused;
        }
        if (events & EVENT_DUMP) {
            save_disassembly_cache();
        }
        // End of frame events
        if(paused)
        {
            render_rgb(&bus.ppu);
            // Skip till next instruction
            if(events & EVENT_STEP) {
                do {
                    step(&bus);
                } while(bus.cpu.cycles>0);
                do {
                    step(&bus);
                } while(bus.cpu.cycles==0);
            }
        }
        sdl_render(&ctx, paused, bus.ppu.renderer.framebuffer_rgb, disassembly_cache, bus.cpu.PC, current_fps);
        if(bus.ppu.vblank_triggered==1) {
            bus.ppu.vblank_triggered=0;
        }
        sync_frame(&last_time_ns);
        fps_frame_count++;
        uint64_t now = SDL_GetTicksNS();
        if (now - fps_last_time >= 1000000000) {
            current_fps = fps_frame_count;
            fps_frame_count = 0;
            fps_last_time = now;
        }
    }
    free_bus(&bus);
    sdl_cleanup(&ctx);
    return 0;
}