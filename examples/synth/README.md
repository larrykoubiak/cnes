# One oscillator, in C

This independent example borrows the audio format and push-stream approach from
`src/frontend/audio.c`: SDL 3.2.8, 44,100 samples/second, unsigned 8-bit mono,
and `SDL_PutAudioStreamData`. It links only SDL; no APU, ROM, ring buffer, or
SDL_ttf. The emulator's source and build are unchanged.

From the repository root (CMake 3.30+, a C compiler, Git, and internet for SDL;
Linux also needs SDL's usual audio and X11/Wayland development packages):

```sh
cmake -S examples/synth -B build-synth
cmake --build build-synth --target synth -j2
./build-synth/synth
```

Focus the small window. Left/right change frequency by 20 Hz; down/up change
volume by 5 percentage points; Escape or closing the window quits. The title
shows the values. It starts at 440 Hz and 10% amplitude. Frequency is limited
to 20–2,000 Hz and volume to 0–100%. Volume here means waveform amplitude,
not a calibrated measure of perceived loudness.

## Read `generate()` first

The whole sound-making part is this relationship:

```c
wave = sin(2 * pi * phase);
sample = 128 + 127 * volume * wave;
phase += frequency / 44100;
```

1. **A sample is one number at one instant.** SDL takes 44,100 of these per
   second. The samples describe a wave; you don't send SDL a “play A” command.
2. **Phase is your position in a repeating cycle.** At phase 0, 0.25, 0.5,
   and 0.75, the sine wave is approximately 0, +1, 0, and -1. `sin` expects
   radians, so multiply the fraction by one full turn: `2*pi`.
3. **Frequency determines how far you advance per sample.** At 440 Hz,
   the step is `440/44100`, about 0.00998 cycles. It takes about 100.23 samples
   to complete a cycle. Fractional phase handles that naturally. After passing
   1, subtract 1; keep the leftover fraction. The frequency cap keeps the step
   below 1, so a single subtraction suffices.
4. **Volume scales the wave around silence.** Unsigned bytes cannot represent
   negative values, so 128 is the midpoint. With volume 0, every byte is 128.
   At full amplitude, this mapping covers 1–255. The code adds 0.5 before
   converting to a byte to round, instead of truncating.

`phase` lives outside `generate()`. A block is merely a convenient batch of
512 samples, not a musical unit. Resetting phase for each block would introduce
discontinuities. Changing frequency changes the step while keeping phase.

## The rest is plumbing

The device starts paused. We queue a block, then resume it. The loop reads keys
and keeps roughly two blocks of input queued (23 ms, plus device/conversion
buffering). For this format one sample is one byte, so queued bytes are also
queued samples. SDL copies the submitted data, allowing reuse of the array.

`SDL_Delay(2)` lets the CPU do other work; it does not time individual samples.
The audio device consumes them at the stream's sample rate. A long scheduling
stall can still cause an underrun; more buffering tolerates stalls but delays
control changes. No callback or shared-thread state is needed here.

This is deliberately a learning example: 8-bit quantization is audible,
and abrupt volume changes/start/stop can click. Smoothing and envelopes can
come later, once the oscillator makes sense.

Try changing 440 to 220: the phase advances half as fast, giving an octave
lower. Then set volume to zero: phase still moves, but all samples become 128.

SDL reference: [opening a device stream](https://wiki.libsdl.org/SDL3/SDL_OpenAudioDeviceStream),
[queued input bytes](https://wiki.libsdl.org/SDL3/SDL_GetAudioStreamQueued),
[submitting samples](https://wiki.libsdl.org/SDL3/SDL_PutAudioStreamData).
