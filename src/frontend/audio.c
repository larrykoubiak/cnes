#include <stdio.h>
#include "frontend/sdl_context.h"

int audio_init(Audio_Frontend* audio) {
    SDL_zero(audio->spec);
    audio->spec.freq     = SAMPLE_RATE;                    // sample rate
    audio->spec.format   = SDL_AUDIO_U8;             // 8-bit unsigned
    audio->spec.channels = 1;                        // mono
    audio->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio->spec,NULL, NULL);
    audio->sample_rate_step = SAMPLE_TIME_NS;
    audio->last_time_ns = 0;
    if (!audio->stream) {
        printf("SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_memset(audio->buffer, 128, 735);
    return 0; // or your existing return logic
}

void audio_update(Audio_Frontend* audio, ring_buffer* rb, bool paused) {
    int fill = ring_buffer_fill(rb);
    uint64_t now = SDL_GetTicksNS();
    if(audio->last_time_ns==0) {
        SDL_ResumeAudioStreamDevice(audio->stream);
        audio->last_time_ns = now;
    }
    uint64_t elapsed = (now - audio->last_time_ns);
    int to_play = (int)(elapsed / audio->sample_rate_step);
    int total = (fill < to_play ? fill : to_play);
    // int total = to_play;
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
    audio->last_time_ns = now;
}

int audio_cleanup(Audio_Frontend* audio) {
    if(audio->stream) {
        SDL_DestroyAudioStream(audio->stream);
    }
}