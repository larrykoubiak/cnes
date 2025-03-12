#ifndef ADDRESSING_MODE_H
#define ADDRESSING_MODE_H

#include <stdint.h>
typedef struct CPU CPU;

uint16_t _addr_ABS(CPU* cpu);
uint16_t _addr_ABS_X(CPU* cpu);
uint16_t _addr_ABS_Y(CPU* cpu);
uint16_t _addr_ACC(CPU* cpu);
uint16_t _addr_IMP(CPU* cpu);
uint16_t _addr_IMM(CPU* cpu);
uint16_t _addr_IND(CPU* cpu);
uint16_t _addr_IND_X(CPU* cpu);
uint16_t _addr_IND_Y(CPU* cpu);
uint16_t _addr_REL(CPU* cpu);
uint16_t _addr_ZP(CPU* cpu);
uint16_t _addr_ZP_X(CPU* cpu);
uint16_t _addr_ZP_Y(CPU* cpu);

#endif