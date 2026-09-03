#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "apu/apu.h"
#include "utils/constants.h"

static int failures = 0;

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "%s:%d: check failed: %s\n",                     \
                    __FILE__, __LINE__, #condition);                            \
            failures++;                                                        \
        }                                                                       \
    } while (0)

static void step_apu(APU *apu, uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; i++) {
        apu_step(apu);
    }
}

static void test_ring_buffer_keeps_newest_samples(void) {
    ring_buffer rb = {0};

    for (uint32_t i = 0; i < APU_SAMPLE_BUFFER_SIZE + 1U; i++) {
        (void)write_rb(&rb, (int16_t)i);
    }

    CHECK(ring_buffer_fill(&rb) == APU_SAMPLE_BUFFER_SIZE);
    CHECK(rb.dropped_samples == 1);
    CHECK(read_rb(&rb) == 1);
    ring_buffer_clear(&rb);
    CHECK(ring_buffer_fill(&rb) == 0);
}

static void test_pulse_register_side_effects(void) {
    PulseChannel pulse = {0};

    pulse_write(&pulse, 0, 0x02);
    CHECK(!pulse.envelope_start_flag);

    pulse_set_enabled(&pulse, true);
    pulse_write(&pulse, 2, 100);
    pulse_write(&pulse, 3, 0x00);
    CHECK(pulse.timer_period == 100);
    CHECK(pulse.length_counter == 10);
    CHECK(pulse.envelope_start_flag);

    pulse_step_envelope(&pulse);
    CHECK(pulse.envelope_volume == 15);
    CHECK(pulse.envelope_divider == 2);
    pulse_step_envelope(&pulse);
    pulse_step_envelope(&pulse);
    CHECK(pulse.envelope_volume == 15);
    pulse_step_envelope(&pulse);
    CHECK(pulse.envelope_volume == 14);

    pulse_set_enabled(&pulse, false);
    CHECK(pulse.length_counter == 0);
    pulse_set_enabled(&pulse, true);
    CHECK(pulse.length_counter == 0);
}

static void test_pulse_sweep_negation(void) {
    PulseChannel pulse1 = {0};
    PulseChannel pulse2 = {0};

    pulse1.timer_period = 1000;
    pulse2.timer_period = 1000;
    pulse_write(&pulse1, 1, 0x89);
    pulse_write(&pulse2, 1, 0x89);
    pulse1.sweep_reload = false;
    pulse2.sweep_reload = false;

    pulse_step_sweep(&pulse1, true);
    pulse_step_sweep(&pulse2, false);

    CHECK(pulse1.timer_period == 499);
    CHECK(pulse2.timer_period == 500);
}

static void test_triangle_timer_and_dac_hold(void) {
    TriangleChannel triangle = {0};

    triangle_set_enabled(&triangle, true);
    triangle_write(&triangle, 0, 0x81);
    CHECK(!triangle.linear_reload_flag);
    triangle_write(&triangle, 2, 9);
    triangle_write(&triangle, 3, 0x00);
    CHECK(triangle.length_counter == 10);
    CHECK(triangle.linear_reload_flag);

    triangle_step(&triangle);
    CHECK(triangle.sequence_index == 0);
    triangle_step_linear(&triangle);
    triangle.timer_counter = 0;
    triangle_step(&triangle);
    CHECK(triangle.sequence_index == 1);
    CHECK(triangle_get_output_amplitude(&triangle) == 14);

    triangle.linear_counter = 0;
    triangle.timer_counter = 0;
    triangle_step(&triangle);
    CHECK(triangle.sequence_index == 1);
    triangle_set_enabled(&triangle, false);
    CHECK(triangle.length_counter == 0);
    CHECK(triangle_get_output_amplitude(&triangle) == 14);
}

static void test_noise_period_uses_cpu_cycles(void) {
    NoiseChannel noise;
    noise_reset(&noise);

    noise_step(&noise);
    uint16_t first_value = noise.shift_register;
    CHECK(first_value == 0x4000);

    noise_step(&noise);
    noise_step(&noise);
    noise_step(&noise);
    CHECK(noise.shift_register == first_value);

    noise_step(&noise);
    CHECK(noise.shift_register != first_value);
}

static void test_frame_counter_rate(void) {
    APU apu;
    apu_init(&apu, SAMPLE_RATE);

    apu_write(&apu, 0x15, 0x01);
    apu_write(&apu, 0x00, 0x00);
    apu_write(&apu, 0x02, 100);
    apu_write(&apu, 0x03, 0x00);

    step_apu(&apu, 7456);
    CHECK(apu.pulse1.envelope_start_flag);
    CHECK(apu.pulse1.length_counter == 10);

    step_apu(&apu, 1);
    CHECK(!apu.pulse1.envelope_start_flag);
    CHECK(apu.pulse1.envelope_volume == 15);
    CHECK(apu.pulse1.length_counter == 10);

    step_apu(&apu, 7455);
    CHECK(apu.pulse1.length_counter == 10);
    step_apu(&apu, 1);
    CHECK(apu.pulse1.envelope_volume == 14);
    CHECK(apu.pulse1.length_counter == 9);
}

static void test_five_step_write_clocks_both_units(void) {
    APU apu;
    apu_init(&apu, SAMPLE_RATE);

    apu_write(&apu, 0x15, 0x01);
    apu_write(&apu, 0x00, 0x00);
    apu_write(&apu, 0x02, 100);
    apu_write(&apu, 0x03, 0x00);
    apu_write(&apu, 0x17, 0x80);

    step_apu(&apu, 2);
    CHECK(apu.pulse1.envelope_start_flag);
    CHECK(apu.pulse1.length_counter == 10);
    step_apu(&apu, 1);
    CHECK(apu.frame_mode == 1);
    CHECK(apu.pulse1.envelope_volume == 15);
    CHECK(apu.pulse1.length_counter == 9);
}

static void test_status_reflects_counters_and_clears_irq(void) {
    APU apu;
    apu_init(&apu, SAMPLE_RATE);

    apu_write(&apu, 0x15, 0x01);
    apu_write(&apu, 0x03, 0x00);
    apu.frame_irq_flag = true;

    uint8_t status = apu_read(&apu, 0x15);
    CHECK((status & 0x01U) != 0);
    CHECK((status & 0x40U) != 0);
    CHECK((apu_read(&apu, 0x15) & 0x40U) == 0);

    apu_write(&apu, 0x15, 0x00);
    CHECK((apu_read(&apu, 0x15) & 0x01U) == 0);
}

static void test_fractional_sample_clock(void) {
    APU apu;
    apu_init(&apu, SAMPLE_RATE);

    step_apu(&apu, 1789773U);
    CHECK(apu.output_sample_count == SAMPLE_RATE);
}

static void test_silence_and_signed_pcm(void) {
    APU apu;
    apu_init(&apu, SAMPLE_RATE);
    step_apu(&apu, 1000);

    while (ring_buffer_fill(&apu.sample_buffer) > 0) {
        CHECK(read_rb(&apu.sample_buffer) == 0);
    }

    apu_write(&apu, 0x15, 0x01);
    apu_write(&apu, 0x00, 0x9F);
    apu_write(&apu, 0x02, 99);
    apu_write(&apu, 0x03, 0x08);
    step_apu(&apu, 100000);

    int16_t minimum = INT16_MAX;
    int16_t maximum = INT16_MIN;
    while (ring_buffer_fill(&apu.sample_buffer) > 0) {
        int16_t sample = read_rb(&apu.sample_buffer);
        if (sample < minimum) {
            minimum = sample;
        }
        if (sample > maximum) {
            maximum = sample;
        }
    }

    CHECK(minimum < 0);
    CHECK(maximum > 0);
}

static void test_pcm_pitch_is_not_tied_to_video_frames(void) {
    APU apu;
    apu_init(&apu, SAMPLE_RATE);

    // Pulse timer 253 gives 1,789,772.7 / (16 * 254) = 440.4 Hz.
    apu_write(&apu, 0x15, 0x01);
    apu_write(&apu, 0x00, 0xBF);
    apu_write(&apu, 0x02, 253);
    apu_write(&apu, 0x03, 0x00);

    step_apu(&apu, 200000);
    ring_buffer_clear(&apu.sample_buffer);

    int positive_crossings = 0;
    int16_t previous = 0;
    bool have_previous = false;
    for (uint32_t cycle = 0; cycle < 1789773U; cycle++) {
        apu_step(&apu);
        while (ring_buffer_fill(&apu.sample_buffer) > 0) {
            int16_t sample = read_rb(&apu.sample_buffer);
            if (have_previous && previous <= 0 && sample > 0) {
                positive_crossings++;
            }
            previous = sample;
            have_previous = true;
        }
    }

    CHECK(positive_crossings >= 439);
    CHECK(positive_crossings <= 442);
}

int main(void) {
    test_ring_buffer_keeps_newest_samples();
    test_pulse_register_side_effects();
    test_pulse_sweep_negation();
    test_triangle_timer_and_dac_hold();
    test_noise_period_uses_cpu_cycles();
    test_frame_counter_rate();
    test_five_step_write_clocks_both_units();
    test_status_reflects_counters_and_clears_irq();
    test_fractional_sample_clock();
    test_silence_and_signed_pcm();
    test_pcm_pitch_is_not_tied_to_video_frames();

    if (failures != 0) {
        fprintf(stderr, "%d APU test(s) failed\n", failures);
        return 1;
    }

    printf("all APU tests passed\n");
    return 0;
}
