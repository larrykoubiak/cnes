#include <stdio.h>
#include "frontend/sdl_context.h"

int audio_init(Audio_Frontend* audio) {
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq     = 44100;                    // sample rate
    desired.format   = SDL_AUDIO_S16;             // 16-bit signed
    desired.channels = 1;                        // mono
    audio->audio_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if (!audio->audio_device) {
        printf("SDL_OpenAudioDevice Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    } else {
        SDL_ResumeAudioDevice(audio->audio_device);
    }

    return 0; // or your existing return logic
}
int audio_update(Audio_Frontend* audio) {
    // TO DO
}

int audio_cleanup(Audio_Frontend* audio) {
    if(audio->audio_device) {
        SDL_CloseAudioDevice(audio->audio_device);
    }
}