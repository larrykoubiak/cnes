#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#include "addressing_modes.h"
#include "instructions.h"

typedef struct Bus Bus;
typedef struct CPU CPU;

typedef uint16_t (*AddressModeFunc)(CPU* cpu);
typedef void (*InstructionFunc)(CPU* cpu, uint16_t address);

typedef struct OpcodeDefinition {
    AddressModeFunc addr_func;
    InstructionFunc instr_func;
    uint8_t         base_cycles;
    const char*     disassembly;
} OpcodeDefinition;

typedef struct CPU {
    uint8_t             A;
    uint8_t             X;
    uint8_t             Y;
    uint16_t            PC;
    uint8_t             SP;
    union {
        struct {
            uint8_t C : 1;
            uint8_t Z : 1;
            uint8_t I : 1;
            uint8_t D : 1;
            uint8_t B : 1;
            uint8_t U : 1;
            uint8_t V : 1;
            uint8_t N : 1;
        };
        uint8_t value;
    } P;
    char                disassembly[16];
    int                 cycles;
    Bus*                bus;
    OpcodeDefinition    opcode_table[256];
    bool                nmi_pending;
} CPU;

void _branch(CPU* cpu, int flag, int check, uint16_t address);
uint8_t _read_PC_incr(CPU* cpu);
void _stack_push(CPU* cpu, uint8_t value);
uint8_t _stack_pull(CPU* cpu);

int cpu_init(CPU* cpu, struct Bus* bus);
void cpu_reset(CPU* cpu);
void cpu_step(CPU* cpu);
void cpu_handle_nmi(CPU* cpu);

#endif