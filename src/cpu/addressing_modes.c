#include "../bus.h"
uint16_t _addr_ABS(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    uint8_t addr_hi = _read_PC_incr(cpu);
    return (uint16_t)((addr_hi << 8) | addr_lo);
}

uint16_t _addr_ABS_X(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    uint8_t addr_hi = _read_PC_incr(cpu);
    uint16_t base   = (uint16_t)((addr_hi << 8) | addr_lo);
    uint16_t new_addr = (uint16_t)(base + cpu->X);
    // if page boundary crossed
    if ((new_addr >> 8) > addr_hi) {
        cpu->cycles += 1;
    }
    return new_addr;
}

uint16_t _addr_ABS_Y(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    uint8_t addr_hi = _read_PC_incr(cpu);
    uint16_t base   = (uint16_t)((addr_hi << 8) | addr_lo);
    uint16_t new_addr = (uint16_t)(base + cpu->Y);
    if ((new_addr >> 8) > addr_hi) {
        cpu->cycles += 1;
    }
    return new_addr;
}

uint16_t _addr_ACC(CPU* cpu)
{
    (void)cpu; // not used
    return 0xFFFF;  // 0xFFFF => "use A register directly"
}

uint16_t _addr_IMP(CPU* cpu)
{
    (void)cpu; 
    return 0xFFFF;  // implied
}

uint16_t _addr_IMM(CPU* cpu)
{
    uint16_t addr = cpu->PC;
    cpu->PC += 1;
    return addr;
}

uint16_t _addr_IND(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    uint8_t addr_hi = _read_PC_incr(cpu);
    uint16_t pointer = (uint16_t)((addr_hi << 8) | addr_lo);
    // 6502 "jump bug": LSB wraps if page boundary crossed
    uint8_t ind_lo = cpu_read(cpu->bus, pointer);
    pointer = (uint16_t)((addr_hi << 8) | ((addr_lo + 1) & 0xFF));
    uint8_t ind_hi = cpu_read(cpu->bus, pointer);
    return (uint16_t)((ind_hi << 8) | ind_lo);
}

uint16_t _addr_IND_X(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    uint8_t pointer = (uint8_t)((addr_lo + cpu->X) & 0xFF);
    uint8_t ind_lo  = cpu_read(cpu->bus, pointer);
    pointer = (uint8_t)((addr_lo + cpu->X + 1) & 0xFF);
    uint8_t ind_hi  = cpu_read(cpu->bus, pointer);
    return (uint16_t)((ind_hi << 8) | ind_lo);
}

uint16_t _addr_IND_Y(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    uint8_t pointer = addr_lo;
    uint8_t ind_lo  = cpu_read(cpu->bus, pointer);
    pointer = (uint8_t)((addr_lo + 1) & 0xFF);
    uint8_t ind_hi  = cpu_read(cpu->bus, pointer);
    uint16_t base   = (uint16_t)((ind_hi << 8) | ind_lo);
    uint16_t new_addr = (uint16_t)(base + cpu->Y);
    if ((new_addr >> 8) > ind_hi) {
        cpu->cycles += 1;
    }
    return new_addr;
}

uint16_t _addr_REL(CPU* cpu)
{
    // signed offset
    int8_t offset = (int8_t)cpu_read(cpu->bus, cpu->PC);
    cpu->PC += 1;
    // add offset to PC
    uint16_t new_addr = (uint16_t)(cpu->PC + offset);
    return new_addr;
}

uint16_t _addr_ZP(CPU* cpu)
{
    return _read_PC_incr(cpu);
}

uint16_t _addr_ZP_X(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    return (uint8_t)(addr_lo + cpu->X);
}

uint16_t _addr_ZP_Y(CPU* cpu)
{
    uint8_t addr_lo = _read_PC_incr(cpu);
    return (uint8_t)(addr_lo + cpu->Y);
}