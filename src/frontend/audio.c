#include <stdio.h>
#include "frontend/sdl_context.h"

int audio_init(Audio_Frontend* audio) {
    SDL_zero(audio->spec);
    audio->spec.freq     = 44100;                    // sample rate
    audio->spec.format   = SDL_AUDIO_U8;             // 8-bit unsigned
    audio->spec.channels = 1;                        // mono
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

int audio_update(Audio_Frontend* audio, ring_buffer* buffer) {
    for(int i=0;i < 735;i++) {
        audio->buffer[i] = read_rb(buffer);
    }
    SDL_PutAudioStreamData(audio->stream, &audio->buffer,735);
}

int audio_cleanup(Audio_Frontend* audio) {
    if(audio->stream) {
        SDL_DestroyAudioStream(audio->stream);
    }
}