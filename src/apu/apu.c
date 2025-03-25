#include "bus.h"

void apu_init(APU* apu, int sample_rate) {
    memset(apu, 0, sizeof(*apu));
    apu->sample_rate = sample_rate;
    apu->sample_rate_step = ((uint64_t)CPU_FREQ_NTSC << APU_FIXED_SHIFT) / apu->sample_rate;
    apu->sample_timer = 0;
    apu->frame_mode = 0;  // default 4-step mode
    apu->frame_irq_inhibit = 0;
    apu->frame_irq_flag = false;
    apu->frame_step = 0;
    apu->frame_sequencer_next = apu->cycle_count + 7457;
}

void apu_reset(APU* apu){
    memset(apu, 0 , sizeof(APU));
}

void apu_write(APU* apu, uint16_t addr, uint8_t value) {
    if(addr < 0x14) {
        uint8_t channel_id = addr >> 2;
        uint8_t field_id = addr & 0x03;
        switch(channel_id) {
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
            }
    } else if(addr==0x15){
        apu->status = value;
        apu->pulse1.enabled = value & 0x01;
        apu->pulse2.enabled = value & 0x02;
        apu->triangle.enabled = value & 0x04;
        apu->noise.enabled = value & 0x08;
        apu->dmc.enabled = value & 0x10;
    } else if(addr==0x17){
        apu->frame_mode = (value >> 7) & 0x01;
        apu->frame_irq_inhibit =(value >> 6) & 0x01;
        if(apu->frame_irq_inhibit)
            apu->frame_irq_flag = false;
        apu->frame_step = 0;
        if(apu->frame_mode == 1) {
            apu->frame_sequencer_next = apu->cycle_count + 1;
            apu_update_frame_sequencer(apu);
        } else {
            apu->frame_sequencer_next = apu->cycle_count + 7457;
        }
    }
}

uint8_t apu_read(APU* apu, uint16_t addr){
    if (addr == 0x15) {
        return apu->status;
    }
    return 0;
}

void apu_step(APU* apu) {
    apu->cycle_count++;
    pulse_step(&apu->pulse1);
    pulse_step(&apu->pulse2);
    triangle_step(&apu->triangle);
    noise_step(&apu->noise);
    if (apu->cycle_count == apu->frame_sequencer_next) {
        apu_update_frame_sequencer(apu);
    }
    apu->sample_timer += (1ULL << APU_FIXED_SHIFT);
    if (apu->sample_timer >= apu->sample_rate_step) {
        apu->sample_timer -= apu->sample_rate_step;
        apu_output_sample(apu);
    }
}

void apu_update_frame_sequencer(APU* apu) {
    bool is_5step = apu->frame_mode;
    int step = apu->frame_step;    
    bool clock_env   = is_5step ? clock_envelope_5step[step] : clock_envelope_4step[step];
    bool clock_len   = is_5step ? clock_length_sweep_5step[step] : clock_length_sweep_4step[step];
    if (clock_env) {
        pulse_step_envelope(&apu->pulse1);
        pulse_step_envelope(&apu->pulse2);
        triangle_step_linear(&apu->triangle);
        noise_step_envelope(&apu->noise);
    }
    if (clock_len) {
        pulse_step_length(&apu->pulse1);
        pulse_step_length(&apu->pulse2);
        pulse_step_sweep(&apu->pulse1, true);
        pulse_step_sweep(&apu->pulse2, false);
        triangle_step_length(&apu->triangle);
        noise_step_length(&apu->noise);
    }
    if (!is_5step && step == 3 && !apu->frame_irq_inhibit) {
        apu->frame_irq_flag = true;
    }
    apu->frame_step++;
    if ((!is_5step && apu->frame_step > 3) ||
        ( is_5step && apu->frame_step > 4)) {
        apu->frame_step = 0;
        apu->frame_sequencer_next = is_5step ? UINT64_MAX
                                                : apu->cycle_count + 7457;
    } else {
        apu->frame_sequencer_next += 7457;
    }
}

void apu_output_sample(APU* apu) {
    int pulse1_amp = pulse_get_output_amplitude(&apu->pulse1);
    int pulse2_amp = pulse_get_output_amplitude(&apu->pulse2);
    int triangle_amp = triangle_get_output_amplitude(&apu->triangle);
    int noise_amp = noise_get_output_amplitude(&apu->noise);
    int dmc_amp = 0;
    // pulse
    float pulse_out = 0.0f;
    int pulse_sum = pulse1_amp + pulse2_amp;
    if (pulse_sum > 0) {
        pulse_out = 95.88f / ((8128.0f / pulse_sum) + 100.0f);
    }
    // triangle + noise + dmc
    float tnd_out = 0.0f;
    float tnd_sum = triangle_amp / 8227.0f +
                    noise_amp    / 12241.0f +
                    dmc_amp      / 22638.0f;
    if (tnd_sum > 0.0f) {
        tnd_out = 159.79f / (1.0f / tnd_sum + 100.0f);
    } 
    //mixing
    float mixed = pulse_out + tnd_out;
    float volume = 0.3f;
    uint8_t sample = (uint8_t)(mixed * 127.5f * volume + 0.5f); // 0.0→0, 1.0→127
    sample += 128;
    write_rb(&apu->sample_buffer, sample);
}