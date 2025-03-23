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
    SDL_zero(audio->buffer);
    audio->read_pos = 0;
    audio->write_pos = 0;
    audio->sample_pos = 0;
    SDL_ResumeAudioStreamDevice(audio->stream);
    return 0; // or your existing return logic
}
int audio_update(Audio_Frontend* audio) {
    // TEST: write sample data
    for(int i=0; i < 735; i++) {
        uint8_t sample = 0;
        if(((audio->sample_pos % 100) / 50)==0) {
            sample += 0x60;
        };
        if((audio->sample_pos / 100)==0) {
            sample += 0x20;
        };
        audio->buffer[audio->write_pos] = sample;
        audio->write_pos++;
        if(audio->write_pos>=1470) {
            audio->write_pos -= 1470;
        }
        audio->sample_pos++;
        if(audio->sample_pos == 200) {
            audio->sample_pos = 0;
        }
    }
    // copy to stream
    int8_t *read_buffer = &audio->buffer[audio->read_pos];
    SDL_PutAudioStreamData(audio->stream, read_buffer,735);
    audio->read_pos += 735;
    if(audio->read_pos >= 1470) {
        audio->read_pos -= 1470;
    }
}

int audio_cleanup(Audio_Frontend* audio) {
    if(audio->stream) {
        SDL_DestroyAudioStream(audio->stream);
    }
}