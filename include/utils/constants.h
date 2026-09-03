#ifndef CONSTANTS_H
#define CONSTANTS_H


// timing
#define TARGET_FPS 60.0988138974405
#define FRAME_TIME_NS 16639263ULL

// NTSC CPU clock: 39,375,000 / 22 Hz. Keeping it as a rational lets the
// sample phase accumulator run without rounding the CPU frequency.
#define NTSC_CPU_CLOCK_NUMERATOR   39375000ULL
#define NTSC_CPU_CLOCK_DENOMINATOR 22ULL

// audio
#define SAMPLE_RATE 44100U
#define APU_SAMPLE_BUFFER_SIZE 8192U
#define MAX_AUDIO_BUFFERED 2048U
#define AUDIO_PRIME_SAMPLES 735U
#define AUDIO_MAX_QUEUED_SAMPLES (AUDIO_PRIME_SAMPLES * 4U)
// events
#define EVENT_QUIT  (1 << 0)
#define EVENT_DUMP  (1 << 1)
#define EVENT_PAUSE (1 << 2)
#define EVENT_STEP  (1 << 3)

#endif
