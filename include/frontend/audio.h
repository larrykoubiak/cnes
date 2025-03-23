#ifndef FRONTEND_AUDIO_H
#define FRONTEND_AUDIO_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_AudioDeviceID audio_device;
} Audio_Frontend;

int audio_init(Audio_Frontend* audio);
int audio_update(Audio_Frontend* audio);
int audio_cleanup(Audio_Frontend* audio);

#endif