#include <stdio.h>
#include "frontend/sdl_context.h"

int audio_init(Audio_Frontend* audio) {
    SDL_zero(audio->spec);
    audio->spec.freq     = 44100;                    // sample rate
    audio->spec.format   = SDL_AUDIO_U8;             // 8-bit unsigned
    audio->spec.channels = 1;                        // mono
    audio->lowpass = 0.0f;
    audio->highpass = 0.0f;
    audio->highpass_prev_input = 0.0f;
    audio->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio->spec,NULL, NULL);
    if (!audio->stream) {
        printf("SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_memset(audio->buffer, 128, 735);
    SDL_ResumeAudioStreamDevice(audio->stream);
    return 0; // or your existing return logic
}

void audio_update(Audio_Frontend* audio, ring_buffer* rb, bool paused) {
    int fill = ring_buffer_fill(rb);
    int buffered = SDL_GetAudioStreamAvailable(audio->stream);
    int total = (fill < SAMPLES_PER_FRAME ? fill : SAMPLES_PER_FRAME);
    if (paused) {
        memset(audio->buffer, 128, total );
        SDL_PutAudioStreamData(audio->stream, audio->buffer, total );
        return;
    }
    for (int i = 0; i < total ; i++) {
        uint8_t sample = read_rb(rb);
        audio->buffer[i] = sample;
    }
    SDL_PutAudioStreamData(audio->stream, audio->buffer, total);
}

int audio_cleanup(Audio_Frontend* audio) {
    if(audio->stream) {
        SDL_DestroyAudioStream(audio->stream);
    }
}