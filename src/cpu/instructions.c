#include "../bus.h"

void _instr_ADC(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint16_t result = cpu->A + val + cpu->P.C;
    cpu->P.C = (result >> 8);
    cpu->P.Z = (result == 0);
    cpu->P.V = ((result ^ cpu->A) & (result ^ val) & 0x80) >> 7;
    cpu->P.N = (result & 0x80) >> 7;
    cpu->A = result & 0xFF;
}

void _instr_AND(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint8_t result = cpu->A & val;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    cpu->A = result & 0xFF;
}

void _instr_ASL(CPU* cpu, uint16_t address) {
    uint8_t val = address == 0xFFFF ? cpu->A : cpu_read(cpu->bus, address);
    uint8_t result = val << 1;
    cpu->P.C = val >> 7 ;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    if(address == 0xFFFF) {
        cpu->A = result & 0xFF;
    }
    else {
        cpu_write(cpu->bus, address, val); // RMW - write the original value first
        cpu_write(cpu->bus, address, result & 0xFF);
    }
}

void _instr_BCC(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.C, 0, address);
}

void _instr_BCS(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.C, 1, address);
}

void _instr_BEQ(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.Z, 1, address);
}

void _instr_BIT(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint8_t result = cpu->A & val;
    cpu->P.Z = (result == 0);
    cpu->P.V = (val & 0x40) >> 6;
    cpu->P.N = (val & 0x80) >> 7;
}

void _instr_BMI(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.N, 1, address);
}

void _instr_BNE(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.Z, 0, address);
}

void _instr_BPL(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.N, 0, address);
}

void _instr_BRK(CPU* cpu, uint16_t address) {
    _stack_push(cpu, cpu->PC >> 8);
    _stack_push(cpu, cpu->PC & 0xFF);
    _stack_push(cpu, cpu->P.value);
    uint8_t addr_lo = cpu_read(cpu->bus, 0xFFFE);
    uint8_t addr_hi = cpu_read(cpu->bus, 0xFFFF);
    uint16_t new_addr = (addr_hi << 8 ) | addr_lo;
    cpu->PC = new_addr;
}

void _instr_BVC(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.V, 0, address);
}

void _instr_BVS(CPU* cpu, uint16_t address) {
    _branch(cpu, cpu->P.V, 1, address);
}

void _instr_CLC(CPU* cpu, uint16_t address) {
    cpu->P.C = 0;
}

void _instr_CLD(CPU* cpu, uint16_t address) {
    cpu->P.D = 0;
}

void _instr_CLI(CPU* cpu, uint16_t address) {
    cpu->P.I = 0;
}

void _instr_CLV(CPU* cpu, uint16_t address) {
    cpu->P.V = 0;
}

void _instr_CMP(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint16_t result = cpu->A - val;
    cpu->P.C = cpu->A >= val;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
}

void _instr_CPX(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint16_t result = cpu->X - val;
    cpu->P.C = cpu->X >= val;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
}

void _instr_CPY(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint16_t result = cpu->Y - val;
    cpu->P.C = cpu->Y >= val;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
}

void _instr_DEC(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint8_t result = val - 1;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    cpu_write(cpu->bus, address, val); // RMW - write the original value first
    cpu_write(cpu->bus, address, result & 0xFF);
}

void _instr_DEX(CPU* cpu, uint16_t address) {
    cpu->X -= 1;
    cpu->P.Z = (cpu->X == 0);
    cpu->P.N = (cpu->X & 0x80) >> 7;
}

void _instr_DEY(CPU* cpu, uint16_t address) {
    cpu->Y -= 1;
    cpu->P.Z = (cpu->Y == 0);
    cpu->P.N = (cpu->Y & 0x80) >> 7;
}

void _instr_EOR(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint8_t result = cpu->A ^ val;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    cpu->A = result & 0xFF;
}

void _instr_INC(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint8_t result = val + 1;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    cpu_write(cpu->bus, address, val); // RMW - write the original value first
    cpu_write(cpu->bus, address, result & 0xFF);
}

void _instr_INX(CPU* cpu, uint16_t address) {
    cpu->X += 1;
    cpu->P.Z = (cpu->X == 0);
    cpu->P.N = (cpu->X & 0x80) >> 7;
}

void _instr_INY(CPU* cpu, uint16_t address) {
    cpu->Y += 1;
    cpu->P.Z = (cpu->Y == 0);
    cpu->P.N = (cpu->Y & 0x80) >> 7;
}

void _instr_JMP(CPU* cpu, uint16_t address) {
    cpu->PC = address;
}

void _instr_JSR(CPU* cpu, uint16_t address) {
    uint16_t return_address = cpu->PC - 1;
    _stack_push(cpu, (return_address >> 8));
    _stack_push(cpu, (return_address & 0xFF));
    cpu->PC = address;
}

void _instr_LDA(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    cpu->P.Z = (val == 0);
    cpu->P.N = (val & 0x80) >> 7;
    cpu->A = val;
}

void _instr_LDX(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    cpu->P.Z = (val == 0);
    cpu->P.N = (val & 0x80) >> 7;
    cpu->X = val;
}

void _instr_LDY(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    cpu->P.Z = (val == 0);
    cpu->P.N = (val & 0x80) >> 7;
    cpu->Y = val;
}

void _instr_LSR(CPU* cpu, uint16_t address) {
    uint8_t val = address == 0xFFFF ? cpu->A : cpu_read(cpu->bus, address);
    uint8_t result = val >> 1;
    cpu->P.C = val & 1;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    if(address == 0xFFFF){
        cpu->A = result & 0xFF;
    } 
    else
    {
        cpu_write(cpu->bus, address, val); // RMW - write the original value first
        cpu_write(cpu->bus, address, result & 0xFF);
    }
}

void _instr_NOP(CPU* cpu, uint16_t address) {
    return;
}

void _instr_ORA(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address);
    uint8_t result = cpu->A | val;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    cpu->A = result & 0xFF;
}

void _instr_PHA(CPU* cpu, uint16_t address) {
    _stack_push(cpu, cpu->A);
}

void _instr_PHP(CPU* cpu, uint16_t address) {
    _stack_push(cpu, cpu->P.value);
}

void _instr_PLA(CPU* cpu, uint16_t address) {
    uint8_t result = _stack_pull(cpu);
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    cpu->A = result;
}

void _instr_PLP(CPU* cpu, uint16_t address) {
    cpu->P.value =_stack_pull(cpu);
}

void _instr_ROL(CPU* cpu, uint16_t address) {
    uint8_t val = address == 0xFFFF ? cpu->A : cpu_read(cpu->bus, address);
    uint8_t result = ((val << 1) & 0xFF) | cpu->P.C;
    cpu->P.C = val >> 7;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    if (address == 0xFFFF) {
        cpu->A = result;
    }
    else 
    {
        cpu_write(cpu->bus, address, val); // RMW - write the original value first
        cpu_write(cpu->bus, address, result);
    }
}

void _instr_ROR(CPU* cpu, uint16_t address) {
    uint8_t val = address == 0xFFFF ? cpu->A : cpu_read(cpu->bus, address);
    uint8_t result = (val >> 1) | (cpu->P.C << 7);
    cpu->P.C = val & 1;
    cpu->P.Z = (result == 0);
    cpu->P.N = (result & 0x80) >> 7;
    if (address == 0xFFFF) {
        cpu->A = result & 0xFF;
    }
    else
    {
        cpu_write(cpu->bus, address, val); //RMW - write the original value first
        cpu_write(cpu->bus, address, result & 0xFF);
    }
}

void _instr_RTI(CPU* cpu, uint16_t address) {
    cpu->P.value = _stack_pull(cpu);
    uint8_t addr_lo = _stack_pull(cpu);
    uint8_t addr_hi = _stack_pull(cpu);
    uint16_t new_addr = (addr_hi << 8 ) | addr_lo;
    cpu->PC = new_addr;
}

void _instr_RTS(CPU* cpu, uint16_t address) {
    uint8_t addr_lo = _stack_pull(cpu);
    uint8_t addr_hi = _stack_pull(cpu);
    uint16_t new_addr = (addr_hi << 8 ) | addr_lo;
    cpu->PC = new_addr + 1;
}

void _instr_SBC(CPU* cpu, uint16_t address) {
    uint8_t val = cpu_read(cpu->bus, address) ^ 0xFF;
    int16_t result = cpu->A - val + cpu->P.C;
    cpu->P.C = ~(result < 0);
    cpu->P.Z = (result == 0);
    cpu->P.V = ((result ^ cpu->A) & (result ^ val) & 0x80) >> 7;
    cpu->P.N = (result & 0x80) >> 7;
    cpu->A = result & 0xFF;
}

void _instr_SEC(CPU* cpu, uint16_t address) {
    cpu->P.C = 1;
}

void _instr_SED(CPU* cpu, uint16_t address) {
    cpu->P.D = 1;
}

void _instr_SEI(CPU* cpu, uint16_t address) {
    cpu->P.I = 1;
}

void _instr_STA(CPU* cpu, uint16_t address) {
    cpu_write(cpu->bus, address, cpu->A);
}

void _instr_STX(CPU* cpu, uint16_t address) {
    cpu_write(cpu->bus, address, cpu->X);
}

void _instr_STY(CPU* cpu, uint16_t address) {
    cpu_write(cpu->bus, address, cpu->Y);
}

void _instr_TAX(CPU* cpu, uint16_t address) {
    cpu->P.Z = (cpu->A == 0);
    cpu->P.N = (cpu->A & 0x80) >> 7;
    cpu->X = cpu->A;
}

void _instr_TAY(CPU* cpu, uint16_t address) {
    cpu->P.Z = (cpu->A == 0);
    cpu->P.N = (cpu->A & 0x80) >> 7;
    cpu->Y = cpu->A;
}

void _instr_TSX(CPU* cpu, uint16_t address) {
    cpu->P.Z = (cpu->SP == 0);
    cpu->P.N = (cpu->SP & 0x80) >> 7;
    cpu->X = cpu->SP;
}

void _instr_TXA(CPU* cpu, uint16_t address) {
    cpu->P.Z = (cpu->X == 0);
    cpu->P.N = (cpu->X & 0x80) >> 7;
    cpu->A = cpu->X;
}

void _instr_TXS(CPU* cpu, uint16_t address) {
    cpu->P.Z = (cpu->X == 0);
    cpu->P.N = (cpu->X & 0x80) >> 7;
    cpu->SP = cpu->X;
}

void _instr_TYA(CPU* cpu, uint16_t address) {
    cpu->P.Z = (cpu->Y == 0);
    cpu->P.N = (cpu->Y & 0x80) >> 7;
    cpu->A = cpu->Y;
}