#include "bus.h"

void apu_init(APU* apu, int sample_rate) {
    memset(apu, 0, sizeof(*apu));
   apu->sample_rate = sample_rate;
    apu->sample_rate_step = ((uint64_t)CPU_FREQ_NTSC << APU_FIXED_SHIFT) / apu->sample_rate;
    apu->sample_timer = 0;
}

void apu_reset(APU* apu){
    memset(apu, 0 , sizeof(APU));
}

void apu_write(APU* apu, uint16_t addr, uint8_t value) {
    if(addr < 0x14) {
        uint8_t reg_id = addr >> 2;
        uint8_t field_id = addr & 0x03;
        switch(reg_id) {
            case 0:
                pulse_write(&apu->pulse1, reg_id, value);
                break;
           case 1:
                pulse_write(&apu->pulse2, reg_id, value);
                break;
            case 2:
                triangle_write(&apu->triangle, reg_id, value);
                break;
            case 3:
                noise_write(&apu->noise, reg_id, value);
                break;
            case 4:
                dmc_write(&apu->dmc, reg_id, value);
                break;
            }
    } else if(addr==0x15){
        apu->status = value;
    } else if(addr==0x17){
        apu->frame_counter = value;
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
    apu->frame_step++;
    if (apu->frame_step > 3) {
        apu->frame_step = 0;
        apu->frame_sequencer_next = apu->cycle_count + 7457;
    } else {
        apu->frame_sequencer_next += 7457;
    }
}

void apu_output_sample(APU* apu) {
    int pulse1_amp    = 8;   // placeholder
    int pulse2_amp    = 8;   // placeholder
    int triangle_amp  = 64;  // placeholder
    int noise_amp     = 8;   // placeholder
    int dmc_amp       = 8;   // placeholder
    int pulses = pulse1_amp + pulse2_amp;  // 0..30 or so
    int tnd    = triangle_amp + noise_amp + dmc_amp; 
    int mixed = pulses * 200 + tnd * 40; 
    if (mixed > 32767)  mixed = 32767;
    if (mixed < -32768) mixed = -32768;
    apu->sample_buffer[apu->sample_write_idx] = (int16_t)mixed;
    apu->sample_write_idx = 
      (apu->sample_write_idx + 1) % APU_SAMPLE_BUFFER_SIZE;
}