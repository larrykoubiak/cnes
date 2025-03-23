#ifndef FRONTEND_AUDIO_H
#define FRONTEND_AUDIO_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_AudioStream* stream;
    SDL_AudioSpec spec;
    uint8_t buffer[1470];
    uint16_t read_pos;
    uint16_t write_pos;
    uint16_t sample_pos;
} Audio_Frontend;

int audio_init(Audio_Frontend* audio);
int audio_update(Audio_Frontend* audio);
int audio_cleanup(Audio_Frontend* audio);

#endif