#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
typedef struct CPU CPU;

void _instr_ADC(CPU* cpu, uint16_t address);
void _instr_AND(CPU* cpu, uint16_t address);
void _instr_ASL(CPU* cpu, uint16_t address);
void _instr_BCC(CPU* cpu, uint16_t address);
void _instr_BCS(CPU* cpu, uint16_t address);
void _instr_BEQ(CPU* cpu, uint16_t address);
void _instr_BIT(CPU* cpu, uint16_t address);
void _instr_BMI(CPU* cpu, uint16_t address);
void _instr_BNE(CPU* cpu, uint16_t address);
void _instr_BPL(CPU* cpu, uint16_t address);
void _instr_BRK(CPU* cpu, uint16_t address);
void _instr_BVC(CPU* cpu, uint16_t address);
void _instr_BVS(CPU* cpu, uint16_t address);
void _instr_CLC(CPU* cpu, uint16_t address);
void _instr_CLD(CPU* cpu, uint16_t address);
void _instr_CLI(CPU* cpu, uint16_t address);
void _instr_CLV(CPU* cpu, uint16_t address);
void _instr_CMP(CPU* cpu, uint16_t address);
void _instr_CPX(CPU* cpu, uint16_t address);
void _instr_CPY(CPU* cpu, uint16_t address);
void _instr_DEC(CPU* cpu, uint16_t address);
void _instr_DEX(CPU* cpu, uint16_t address);
void _instr_DEY(CPU* cpu, uint16_t address);
void _instr_EOR(CPU* cpu, uint16_t address);
void _instr_INC(CPU* cpu, uint16_t address);
void _instr_INX(CPU* cpu, uint16_t address);
void _instr_INY(CPU* cpu, uint16_t address);
void _instr_JMP(CPU* cpu, uint16_t address);
void _instr_JSR(CPU* cpu, uint16_t address);
void _instr_LDA(CPU* cpu, uint16_t address);
void _instr_LDX(CPU* cpu, uint16_t address);
void _instr_LDY(CPU* cpu, uint16_t address);
void _instr_LSR(CPU* cpu, uint16_t address);
void _instr_NOP(CPU* cpu, uint16_t address);
void _instr_ORA(CPU* cpu, uint16_t address);
void _instr_PHA(CPU* cpu, uint16_t address);
void _instr_PHP(CPU* cpu, uint16_t address);
void _instr_PLA(CPU* cpu, uint16_t address);
void _instr_PLP(CPU* cpu, uint16_t address);
void _instr_ROL(CPU* cpu, uint16_t address);
void _instr_ROR(CPU* cpu, uint16_t address);
void _instr_RTI(CPU* cpu, uint16_t address);
void _instr_RTS(CPU* cpu, uint16_t address);
void _instr_SBC(CPU* cpu, uint16_t address);
void _instr_SEC(CPU* cpu, uint16_t address);
void _instr_SED(CPU* cpu, uint16_t address);
void _instr_SEI(CPU* cpu, uint16_t address);
void _instr_STA(CPU* cpu, uint16_t address);
void _instr_STX(CPU* cpu, uint16_t address);
void _instr_STY(CPU* cpu, uint16_t address);
void _instr_TAX(CPU* cpu, uint16_t address);
void _instr_TAY(CPU* cpu, uint16_t address);
void _instr_TSX(CPU* cpu, uint16_t address);
void _instr_TXA(CPU* cpu, uint16_t address);
void _instr_TXS(CPU* cpu, uint16_t address);
void _instr_TYA(CPU* cpu, uint16_t address);

#endif