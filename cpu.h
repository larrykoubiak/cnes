#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

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
    uint8_t         A;
    uint8_t         X;
    uint8_t         Y;
    uint16_t        PC;
    uint8_t         SP;
    union {
        struct {
            unsigned int C : 1;
            unsigned int Z : 1;
            unsigned int I : 1;
            unsigned int D : 1;
            unsigned int B : 1;
            unsigned int U : 1;
            unsigned int V : 1;
            unsigned int N : 1;
        };
        uint8_t value;
    } P;
    char            disassembly[16];
    int             cycles;
    struct Bus* bus;
    OpcodeDefinition opcode_table[256];
    bool            nmi_pending;
} CPU;

int cpu_init(CPU* cpu, struct Bus* bus);
void cpu_reset(CPU* cpu);
int  cpu_step(CPU* cpu);
void cpu_handle_nmi(CPU* cpu);

#endif // CPU_H