#include "bus.h"

void apu_reset(APU* apu){
    memset(apu, 0 , sizeof(APU));
}

void apu_write(APU* apu, uint16_t addr, uint8_t value) {
    uint8_t reg_id = (addr & 0x1C0) >> 2;
    uint8_t field_id = addr & 0x03;
    switch(reg_id) {
        case 1:
            apu->pulse1.raw[field_id] = value;
            break;
       case 2:
            apu->pulse2.raw[field_id] = value;
            break;
        case 3:
            apu->triangle.raw[field_id] = value;
            break;
        case 4:
            apu->noise.raw[field_id] = value;
            break;
        case 5:
            apu->dmc.raw[field_id] = value;
            break;
        case 0x15:
            apu->status = value;
            break;
        case 0x17:
            apu->frame_counter = value;
            break;
    }
}

uint8_t apu_read(APU* apu, uint16_t addr){
    if (addr == 0x15) {
        return apu->status;
    }
    return 0;
}
