#ifndef CONSTANTS_H
#define CONSTANTS_H

// audio
#define APU_FIXED_SHIFT 24
#define CPU_FREQ_NTSC 1789773
#define SAMPLES_PER_FRAME 77
#define APU_SAMPLE_BUFFER_SIZE (SAMPLES_PER_FRAME * 4)
#define MAX_AUDIO_BUFFERED     (SAMPLES_PER_FRAME * 2)
// timing
#define TARGET_FPS 60
#define FRAME_TIME_NS (1000000000ULL / TARGET_FPS)
// events
#define EVENT_QUIT  (1 << 0)
#define EVENT_DUMP  (1 << 1)
#define EVENT_PAUSE (1 << 2)
#define EVENT_STEP  (1 << 3)

#endif