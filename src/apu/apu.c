#include <stddef.h>
#include <string.h>

#include "apu/apu.h"
#include "utils/constants.h"

#define APU_FOUR_STEP_QUARTER_1 7457U
#define APU_FOUR_STEP_HALF_1 14913U
#define APU_FOUR_STEP_QUARTER_2 22371U
#define APU_FOUR_STEP_IRQ_1 29828U
#define APU_FOUR_STEP_HALF_2 29829U
#define APU_FOUR_STEP_END 29830U

#define APU_FIVE_STEP_QUARTER_1 7457U
#define APU_FIVE_STEP_HALF_1 14913U
#define APU_FIVE_STEP_QUARTER_2 22371U
#define APU_FIVE_STEP_HALF_2 37281U
#define APU_FIVE_STEP_END 37282U

#define PI_F 3.14159265358979323846f

static float high_pass_coefficient(float cutoff, uint32_t sample_rate) {
    float dt = 1.0f / (float)sample_rate;
    float rc = 1.0f / (2.0f * PI_F * cutoff);
    return rc / (rc + dt);
}

static float low_pass_coefficient(float cutoff, uint32_t sample_rate) {
    float dt = 1.0f / (float)sample_rate;
    float rc = 1.0f / (2.0f * PI_F * cutoff);
    return dt / (rc + dt);
}

static void apu_filter_init(APUFilter *filter, uint32_t sample_rate) {
    memset(filter, 0, sizeof(*filter));
    filter->high_pass_90_coefficient = high_pass_coefficient(90.0f, sample_rate);
    filter->high_pass_440_coefficient = high_pass_coefficient(440.0f, sample_rate);
    filter->low_pass_14000_coefficient = low_pass_coefficient(14000.0f, sample_rate);
}

static float high_pass(float input, float coefficient,
                       float *previous_input, float *previous_output) {
    float output = coefficient * (*previous_output + input - *previous_input);
    *previous_input = input;
    *previous_output = output;
    return output;
}

static float apu_filter_sample(APUFilter *filter, float input) {
    float output = high_pass(input, filter->high_pass_90_coefficient,
                             &filter->high_pass_90_previous_input,
                             &filter->high_pass_90_previous_output);
    output = high_pass(output, filter->high_pass_440_coefficient,
                       &filter->high_pass_440_previous_input,
                       &filter->high_pass_440_previous_output);

    filter->low_pass_14000_output += filter->low_pass_14000_coefficient *
                                     (output - filter->low_pass_14000_output);
    return filter->low_pass_14000_output;
}

static bool apu_apply_pending_frame_counter_write(APU *apu) {
    if (apu->frame_reset_delay == 0) {
        return false;
    }

    apu->frame_reset_delay--;
    if (apu->frame_reset_delay != 0) {
        return false;
    }

    apu->frame_mode = apu->pending_frame_mode;
    apu->frame_cycle = 0;

    if (apu->frame_mode != 0) {
        // Five-step mode clocks both units when the delayed write takes effect.
        apu_clock_quarter_frame(apu);
        apu_clock_half_frame(apu);
    }

    return true;
}

void apu_init(APU *apu, int sample_rate) {
    memset(apu, 0, sizeof(*apu));
    apu->sample_rate = sample_rate > 0 ? (uint32_t)sample_rate : SAMPLE_RATE;
    noise_reset(&apu->noise);
    apu_filter_init(&apu->filter, apu->sample_rate);
}

void apu_reset(APU *apu) {
    uint32_t sample_rate = apu->sample_rate != 0 ? apu->sample_rate : SAMPLE_RATE;
    apu_init(apu, (int)sample_rate);
}

void apu_write(APU *apu, uint16_t addr, uint8_t value) {
    if (addr < 0x14U) {
        uint8_t channel_id = (uint8_t)(addr >> 2);
        uint8_t field_id = addr & 0x03U;

        switch (channel_id) {
            case 0:
                pulse_write(&apu->pulse1, field_id, value);
                break;
            case 1:
                pulse_write(&apu->pulse2, field_id, value);
                break;
            case 2:
                triangle_write(&apu->triangle, field_id, value);
                break;
            case 3:
                noise_write(&apu->noise, field_id, value);
                break;
            case 4:
                dmc_write(&apu->dmc, field_id, value);
                break;
            default:
                break;
        }
        return;
    }

    if (addr == 0x15U) {
        pulse_set_enabled(&apu->pulse1, (value & 0x01U) != 0);
        pulse_set_enabled(&apu->pulse2, (value & 0x02U) != 0);
        triangle_set_enabled(&apu->triangle, (value & 0x04U) != 0);
        noise_set_enabled(&apu->noise, (value & 0x08U) != 0);

        apu->dmc.enabled = (value & 0x10U) != 0;
        if (!apu->dmc.enabled) {
            apu->dmc.current_length = 0;
        }
        apu->dmc.irq_pending = false;
        return;
    }

    if (addr == 0x17U) {
        apu->pending_frame_mode = (value >> 7) & 0x01U;
        apu->frame_irq_inhibit = (value & 0x40U) != 0;
        if (apu->frame_irq_inhibit) {
            apu->frame_irq_flag = false;
        }

        // The sequencer reset takes effect 3 or 4 CPU cycles after the write,
        // depending on which CPU/APU phase the write lands on.
        apu->frame_reset_delay = (apu->cycle_count & 1U) != 0 ? 4U : 3U;
    }
}

uint8_t apu_read(APU *apu, uint16_t addr) {
    if (addr != 0x15U) {
        return 0;
    }

    uint8_t status = 0;
    status |= apu->pulse1.length_counter > 0 ? 0x01U : 0;
    status |= apu->pulse2.length_counter > 0 ? 0x02U : 0;
    status |= apu->triangle.length_counter > 0 ? 0x04U : 0;
    status |= apu->noise.length_counter > 0 ? 0x08U : 0;
    status |= apu->dmc.current_length > 0 ? 0x10U : 0;
    status |= apu->frame_irq_flag ? 0x40U : 0;
    status |= apu->dmc.irq_pending ? 0x80U : 0;

    apu->frame_irq_flag = false;
    return status;
}

void apu_clock_quarter_frame(APU *apu) {
    pulse_step_envelope(&apu->pulse1);
    pulse_step_envelope(&apu->pulse2);
    triangle_step_linear(&apu->triangle);
    noise_step_envelope(&apu->noise);
}

void apu_clock_half_frame(APU *apu) {
    pulse_step_length(&apu->pulse1);
    pulse_step_length(&apu->pulse2);
    triangle_step_length(&apu->triangle);
    noise_step_length(&apu->noise);
    pulse_step_sweep(&apu->pulse1, true);
    pulse_step_sweep(&apu->pulse2, false);
}

void apu_update_frame_sequencer(APU *apu) {
    if (apu->frame_mode == 0) {
        if (apu->frame_cycle >= APU_FOUR_STEP_IRQ_1 &&
            apu->frame_cycle <= APU_FOUR_STEP_END &&
            !apu->frame_irq_inhibit) {
            apu->frame_irq_flag = true;
        }

        switch (apu->frame_cycle) {
            case APU_FOUR_STEP_QUARTER_1:
            case APU_FOUR_STEP_QUARTER_2:
                apu_clock_quarter_frame(apu);
                break;

            case APU_FOUR_STEP_HALF_1:
            case APU_FOUR_STEP_HALF_2:
                apu_clock_quarter_frame(apu);
                apu_clock_half_frame(apu);
                break;

            default:
                break;
        }

        if (apu->frame_cycle >= APU_FOUR_STEP_END) {
            apu->frame_cycle = 0;
        }
        return;
    }

    switch (apu->frame_cycle) {
        case APU_FIVE_STEP_QUARTER_1:
        case APU_FIVE_STEP_QUARTER_2:
            apu_clock_quarter_frame(apu);
            break;

        case APU_FIVE_STEP_HALF_1:
        case APU_FIVE_STEP_HALF_2:
            apu_clock_quarter_frame(apu);
            apu_clock_half_frame(apu);
            break;

        default:
            break;
    }

    if (apu->frame_cycle >= APU_FIVE_STEP_END) {
        apu->frame_cycle = 0;
    }
}

float apu_mix(const APU *apu) {
    uint8_t pulse1 = pulse_get_output_amplitude(&apu->pulse1);
    uint8_t pulse2 = pulse_get_output_amplitude(&apu->pulse2);
    uint8_t triangle = triangle_get_output_amplitude(&apu->triangle);
    uint8_t noise = noise_get_output_amplitude(&apu->noise);
    uint8_t dmc = dmc_get_output_amplitude(&apu->dmc);

    float pulse_out = 0.0f;
    uint8_t pulse_sum = pulse1 + pulse2;
    if (pulse_sum > 0) {
        pulse_out = 95.88f / ((8128.0f / (float)pulse_sum) + 100.0f);
    }

    float tnd_input = (float)triangle / 8227.0f +
                      (float)noise / 12241.0f +
                      (float)dmc / 22638.0f;
    float tnd_out = 0.0f;
    if (tnd_input > 0.0f) {
        tnd_out = 159.79f / ((1.0f / tnd_input) + 100.0f);
    }

    return pulse_out + tnd_out;
}

void apu_output_sample(APU *apu) {
    float averaged = apu->sample_sum_count > 0
        ? (float)(apu->sample_sum / (double)apu->sample_sum_count)
        : apu_mix(apu);

    apu->sample_sum = 0.0;
    apu->sample_sum_count = 0;

    float filtered = apu_filter_sample(&apu->filter, averaged);
    if (filtered > 1.0f) {
        filtered = 1.0f;
    } else if (filtered < -1.0f) {
        filtered = -1.0f;
    }

    int16_t sample = filtered >= 0.0f
        ? (int16_t)(filtered * 32767.0f)
        : (int16_t)(filtered * 32768.0f);

    (void)write_rb(&apu->sample_buffer, sample);
    apu->output_sample_count++;
}

void apu_step(APU *apu) {
    apu->cycle_count++;

    // Triangle and noise timers run at the CPU clock. Pulse timers run at
    // half the CPU clock. DMC is clocked here too, although sample fetching is
    // still intentionally unimplemented.
    triangle_step(&apu->triangle);
    noise_step(&apu->noise);
    dmc_step(&apu->dmc);
    if ((apu->cycle_count & 1U) != 0) {
        pulse_step(&apu->pulse1);
        pulse_step(&apu->pulse2);
    }

    if (!apu_apply_pending_frame_counter_write(apu)) {
        apu->frame_cycle++;
        apu_update_frame_sequencer(apu);
    }

    apu->sample_sum += apu_mix(apu);
    apu->sample_sum_count++;
    apu->sample_phase += (uint64_t)apu->sample_rate * NTSC_CPU_CLOCK_DENOMINATOR;

    if (apu->sample_phase >= NTSC_CPU_CLOCK_NUMERATOR) {
        apu->sample_phase -= NTSC_CPU_CLOCK_NUMERATOR;
        apu_output_sample(apu);
    }
}
