#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

enum { SAMPLE_RATE = 44100, BLOCK_SIZE = 512 };

/* One oscillator. Phase is a fraction of a cycle: 0=start, 0.5=halfway.
   Keep it between blocks! Restarting each block would break the waveform. */
static void generate(Uint8 *samples, double *phase, double frequency, double volume)
{
    const double step = frequency / SAMPLE_RATE; /* cycles per sample */
    const double tau = 6.283185307179586;         /* radians per cycle */
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        double wave = SDL_sin(tau * *phase);     /* smooth wave, -1 to +1 */
        /* U8 silence is 128, NOT zero. Scale around that midpoint.
           volume 0..1 gives a rounded result in 1..255 (no overflow). */
        samples[i] = (Uint8)(128.0 + 127.0 * volume * wave + 0.5);
        *phase += step;
        if (*phase >= 1.0) *phase -= 1.0;        /* start the next cycle */
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int result = 1;
    SDL_Window *window = NULL;
    SDL_AudioStream *stream = NULL;
    double phase = 0.0, frequency = 440.0, volume = 0.1;
    Uint8 samples[BLOCK_SIZE];
    SDL_AudioSpec spec = { .format = SDL_AUDIO_U8, .channels = 1,
                           .freq = SAMPLE_RATE };

    /* The window only receives keys; no renderer, fonts, or NES code. */
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) goto error;
    window = SDL_CreateWindow("Synth: arrows = pitch/volume, Esc = quit", 640, 120, 0);
    if (!window) goto error;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                      &spec, NULL, NULL);
    if (!stream) goto error;

    /* Queue a little sound before unpausing the device. */
    generate(samples, &phase, frequency, volume);
    if (!SDL_PutAudioStreamData(stream, samples, sizeof samples)) goto error;
    if (!SDL_ResumeAudioStreamDevice(stream)) goto error;
    puts("Focus the window. Left/right: -/+20 Hz; down/up: -/+5% volume; Esc: quit.");

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type != SDL_EVENT_KEY_DOWN) continue;
            switch (event.key.key) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_LEFT:  frequency -= 20.0; break;
                case SDLK_RIGHT: frequency += 20.0; break;
                case SDLK_DOWN:  volume -= 0.05; break;
                case SDLK_UP:    volume += 0.05; break;
                default: break;
            }
            frequency = SDL_clamp(frequency, 20.0, 2000.0);
            volume = SDL_clamp(volume, 0.0, 1.0);
        }
        if (!running) break;
        char title[128];
        SDL_snprintf(title, sizeof title, "%.0f Hz | %.0f%% | arrows: pitch/volume | Esc: quit",
                     frequency, volume * 100.0);
        SDL_SetWindowTitle(window, title);

        /* U8 mono = one byte per sample. Two blocks are about 23 ms.
           Only refill when needed: an unlimited queue would delay controls.
           SDL consumes queued audio independently of this main loop. */
        int queued = SDL_GetAudioStreamQueued(stream);
        if (queued < 0) goto error;
        while (queued < 2 * BLOCK_SIZE) {
            generate(samples, &phase, frequency, volume);
            if (!SDL_PutAudioStreamData(stream, samples, sizeof samples)) goto error;
            queued += sizeof samples;
        }
        SDL_Delay(2); /* Yield CPU; sample rate, not this delay, sets pitch. */
    }
    result = 0;
    goto cleanup;

error:
    fprintf(stderr, "SDL: %s\n", SDL_GetError());
cleanup:
    if (stream) SDL_DestroyAudioStream(stream); /* also closes its device */
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
