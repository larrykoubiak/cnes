#ifndef CONSTANTS_H
#define CONSTANTS_H

// audio
#define APU_SAMPLE_BUFFER_SIZE 1470
#define APU_FIXED_SHIFT 24
#define CPU_FREQ_NTSC 1789773
// timing
#define FPS 60
#define FRAME_TIME (1000 / FPS)  // ~16.67ms per frame
// events
#define EVENT_QUIT  (1 << 0)
#define EVENT_DUMP  (1 << 1)
#define EVENT_PAUSE (1 << 2)
#define EVENT_STEP  (1 << 3)

#endif