#ifndef FRONTEND_AUDIO_H
#define FRONTEND_AUDIO_H

#include <SDL3/SDL.h>
#include "utils/ring_buffer.h"

typedef struct {
    SDL_AudioStream* stream;
    SDL_AudioSpec spec;
    uint8_t buffer[MAX_AUDIO_BUFFERED];
} Audio_Frontend;

int audio_init(Audio_Frontend* audio);
void audio_update(Audio_Frontend* audio, ring_buffer* buffer, bool paused);
int audio_cleanup(Audio_Frontend* audio);

#endif