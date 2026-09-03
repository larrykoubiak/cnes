#include <stdio.h>
#include <string.h>

#include "frontend/audio.h"

int audio_init(Audio_Frontend *audio) {
    memset(audio, 0, sizeof(*audio));
    audio->spec.freq = SAMPLE_RATE;
    audio->spec.format = SDL_AUDIO_S16;
    audio->spec.channels = 1;

    audio->stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio->spec, NULL, NULL);
    if (audio->stream == NULL) {
        fprintf(stderr, "SDL_OpenAudioDeviceStream error: %s\n", SDL_GetError());
        return -1;
    }

    // Leave the device paused until the first real block has been queued. This
    // avoids an immediate startup underrun.
    return 0;
}

void audio_update(Audio_Frontend *audio, ring_buffer *rb, bool paused) {
    if (paused) {
        ring_buffer_clear(rb);
        if (audio->started) {
            if (!SDL_PauseAudioStreamDevice(audio->stream)) {
                fprintf(stderr, "SDL_PauseAudioStreamDevice error: %s\n", SDL_GetError());
            }
            if (!SDL_ClearAudioStream(audio->stream)) {
                fprintf(stderr, "SDL_ClearAudioStream error: %s\n", SDL_GetError());
            }
            audio->started = false;
        }
        return;
    }

    size_t fill = ring_buffer_fill(rb);
    while (fill > MAX_AUDIO_BUFFERED) {
        (void)read_rb(rb);
        fill--;
    }

    size_t total = fill;
    for (size_t i = 0; i < total; i++) {
        audio->buffer[i] = read_rb(rb);
    }

    if (total == 0) {
        return;
    }

    int queued_bytes = SDL_GetAudioStreamQueued(audio->stream);
    if (queued_bytes < 0) {
        fprintf(stderr, "SDL_GetAudioStreamQueued error: %s\n", SDL_GetError());
        return;
    }

    size_t queued_samples = (size_t)queued_bytes / sizeof(int16_t);
    size_t pending_samples = total + (audio->started ? 0U : AUDIO_PRIME_SAMPLES);
    if (queued_samples + pending_samples > AUDIO_MAX_QUEUED_SAMPLES) {
        // The emulation clock is ahead of playback. Dropping this block keeps
        // input latency bounded and lets the device catch up.
        return;
    }

    if (!audio->started) {
        static const int16_t silence[AUDIO_PRIME_SAMPLES] = {0};
        if (!SDL_PutAudioStreamData(audio->stream, silence, (int)sizeof(silence))) {
            fprintf(stderr, "SDL_PutAudioStreamData error: %s\n", SDL_GetError());
            return;
        }
    }

    int byte_count = (int)(total * sizeof(audio->buffer[0]));
    if (!SDL_PutAudioStreamData(audio->stream, audio->buffer, byte_count)) {
        fprintf(stderr, "SDL_PutAudioStreamData error: %s\n", SDL_GetError());
        return;
    }

    if (!audio->started) {
        if (!SDL_ResumeAudioStreamDevice(audio->stream)) {
            fprintf(stderr, "SDL_ResumeAudioStreamDevice error: %s\n", SDL_GetError());
            return;
        }
        audio->started = true;
    }
}

void audio_cleanup(Audio_Frontend *audio) {
    if (audio->stream != NULL) {
        SDL_DestroyAudioStream(audio->stream);
        audio->stream = NULL;
    }
}
