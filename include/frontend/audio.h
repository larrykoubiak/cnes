#ifndef FRONTEND_AUDIO_H
#define FRONTEND_AUDIO_H

#include <SDL3/SDL.h>
#include "utils/ring_buffer.h"

typedef struct {
    SDL_AudioStream* stream;
    SDL_AudioSpec spec;
    int16_t buffer[MAX_AUDIO_BUFFERED];
    bool started;
} Audio_Frontend;

int audio_init(Audio_Frontend* audio);
void audio_update(Audio_Frontend* audio, ring_buffer* buffer, bool paused);
void audio_cleanup(Audio_Frontend* audio);

#endif
