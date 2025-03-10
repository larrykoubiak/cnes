#include "../bus.h"

static inline OpcodeDefinition _new_opcode(AddressModeFunc addr, InstructionFunc instr, uint8_t cycles, const char* disassembly)
{
    OpcodeDefinition opcode;
    opcode.addr_func    = addr;
    opcode.instr_func   = instr;
    opcode.base_cycles  = cycles;
    opcode.disassembly  = disassembly;
    return opcode;
}

static void init_opcode_table(CPU* cpu)
{
    // Initialize all 256 entries to something benign
    for (int i = 0; i < 256; i++) {
        cpu->opcode_table[i] = _new_opcode(NULL, NULL, 0, "");
    }

    // Now set each opcode the same way your .pyx does:
    cpu->opcode_table[0x00] = _new_opcode(_addr_IMM, _instr_BRK, 7,"BRK #$%02X");
    cpu->opcode_table[0x01] = _new_opcode(_addr_IND_X, _instr_ORA, 6,"ORA ($%04X,X)");
    cpu->opcode_table[0x05] = _new_opcode(_addr_ZP, _instr_ORA, 3,"ORA $%02X");
    cpu->opcode_table[0x06] = _new_opcode(_addr_ZP, _instr_ASL, 5,"ASL $%02X");
    cpu->opcode_table[0x08] = _new_opcode(_addr_IMP, _instr_PHP, 3,"PHP");
    cpu->opcode_table[0x09] = _new_opcode(_addr_IMM, _instr_ORA, 2,"ORA #$%02X");
    cpu->opcode_table[0x0A] = _new_opcode(_addr_ACC, _instr_ASL, 2,"ASL A");
    cpu->opcode_table[0x0D] = _new_opcode(_addr_ABS, _instr_ORA, 4,"ORA $%04X");
    cpu->opcode_table[0x0E] = _new_opcode(_addr_ABS, _instr_ASL, 6,"ASL $%04X");
    cpu->opcode_table[0x10] = _new_opcode(_addr_REL, _instr_BPL, 2,"BPL $%04X");
    cpu->opcode_table[0x11] = _new_opcode(_addr_IND_Y, _instr_ORA, 5,"ORA ($%04X),Y");
    cpu->opcode_table[0x15] = _new_opcode(_addr_ZP_X, _instr_ORA, 4,"ORA $%02X,X");
    cpu->opcode_table[0x16] = _new_opcode(_addr_ZP_X, _instr_ASL, 6,"ASL $%02X,X");
    cpu->opcode_table[0x18] = _new_opcode(_addr_IMP, _instr_CLC, 2,"CLC");
    cpu->opcode_table[0x19] = _new_opcode(_addr_ABS_Y, _instr_ORA, 4,"ORA $%04X,Y");
    cpu->opcode_table[0x1D] = _new_opcode(_addr_ABS_X, _instr_ORA, 4,"ORA $%04X,X");
    cpu->opcode_table[0x1E] = _new_opcode(_addr_ABS_X, _instr_ASL, 7,"ASL $%04X,X");
    cpu->opcode_table[0x20] = _new_opcode(_addr_ABS, _instr_JSR, 6,"JSR $%04X");
    cpu->opcode_table[0x21] = _new_opcode(_addr_IND_X, _instr_AND, 6,"AND ($%04X,X)");
    cpu->opcode_table[0x24] = _new_opcode(_addr_ZP, _instr_BIT, 3,"BIT $%02X");
    cpu->opcode_table[0x25] = _new_opcode(_addr_ZP, _instr_AND, 3,"AND $%02X");
    cpu->opcode_table[0x26] = _new_opcode(_addr_ZP, _instr_ROL, 5,"ROL $%02X");
    cpu->opcode_table[0x28] = _new_opcode(_addr_IMP, _instr_PLP, 4,"PLP");
    cpu->opcode_table[0x29] = _new_opcode(_addr_IMM, _instr_AND, 2,"AND #$%02X");
    cpu->opcode_table[0x2A] = _new_opcode(_addr_ACC, _instr_ROL, 2,"ROL A");
    cpu->opcode_table[0x2C] = _new_opcode(_addr_ABS, _instr_BIT, 4,"BIT $%04X");
    cpu->opcode_table[0x2D] = _new_opcode(_addr_ABS, _instr_AND, 4,"AND $%04X");
    cpu->opcode_table[0x2E] = _new_opcode(_addr_ABS, _instr_ROL, 6,"ROL $%04X");
    cpu->opcode_table[0x30] = _new_opcode(_addr_REL, _instr_BMI, 2,"BMI $%04X");
    cpu->opcode_table[0x31] = _new_opcode(_addr_IND_Y, _instr_AND, 5,"AND ($%04X),Y");
    cpu->opcode_table[0x35] = _new_opcode(_addr_ZP_X, _instr_AND, 4,"AND $%02X,X");
    cpu->opcode_table[0x36] = _new_opcode(_addr_ZP_X, _instr_ROL, 6,"ROL $%02X,X");
    cpu->opcode_table[0x38] = _new_opcode(_addr_IMP, _instr_SEC, 2,"SEC");
    cpu->opcode_table[0x39] = _new_opcode(_addr_ABS_Y, _instr_AND, 4,"AND $%04X,Y");
    cpu->opcode_table[0x3D] = _new_opcode(_addr_ABS_X, _instr_AND, 4,"AND $%04X,X");
    cpu->opcode_table[0x3E] = _new_opcode(_addr_ABS_X, _instr_ROL, 7,"ROL $%04X,X");
    cpu->opcode_table[0x40] = _new_opcode(_addr_IMP, _instr_RTI, 6,"RTI");
    cpu->opcode_table[0x41] = _new_opcode(_addr_IND_X, _instr_EOR, 6,"EOR ($%04X,X)");
    cpu->opcode_table[0x45] = _new_opcode(_addr_ZP, _instr_EOR, 3,"EOR $%02X");
    cpu->opcode_table[0x46] = _new_opcode(_addr_ZP, _instr_LSR, 5,"LSR $%02X");
    cpu->opcode_table[0x48] = _new_opcode(_addr_IMP, _instr_PHA, 3,"PHA");
    cpu->opcode_table[0x49] = _new_opcode(_addr_IMM, _instr_EOR, 2,"EOR #$%02X");
    cpu->opcode_table[0x4A] = _new_opcode(_addr_ACC, _instr_LSR, 2,"LSR A");
    cpu->opcode_table[0x4C] = _new_opcode(_addr_ABS, _instr_JMP, 3,"JMP $%04X");
    cpu->opcode_table[0x4D] = _new_opcode(_addr_ABS, _instr_EOR, 4,"EOR $%04X");
    cpu->opcode_table[0x4E] = _new_opcode(_addr_ABS, _instr_LSR, 6,"LSR $%04X");
    cpu->opcode_table[0x50] = _new_opcode(_addr_REL, _instr_BVC, 2,"BVC $%04X");
    cpu->opcode_table[0x51] = _new_opcode(_addr_IND_Y, _instr_EOR, 5,"EOR ($%04X),Y");
    cpu->opcode_table[0x55] = _new_opcode(_addr_ZP_X, _instr_EOR, 4,"EOR $%02X,X");
    cpu->opcode_table[0x56] = _new_opcode(_addr_ZP_X, _instr_LSR, 6,"LSR $%02X,X");
    cpu->opcode_table[0x58] = _new_opcode(_addr_IMP, _instr_CLI, 2,"CLI");
    cpu->opcode_table[0x59] = _new_opcode(_addr_ABS_Y, _instr_EOR, 4,"EOR $%04X,Y");
    cpu->opcode_table[0x5D] = _new_opcode(_addr_ABS_X, _instr_EOR, 4,"EOR $%04X,X");
    cpu->opcode_table[0x5E] = _new_opcode(_addr_ABS_X, _instr_LSR, 7,"LSR $%04X,X");
    cpu->opcode_table[0x60] = _new_opcode(_addr_IMP, _instr_RTS, 6,"RTS");
    cpu->opcode_table[0x61] = _new_opcode(_addr_IND_X, _instr_ADC, 6,"ADC ($%04X,X)");
    cpu->opcode_table[0x65] = _new_opcode(_addr_ZP, _instr_ADC, 3,"ADC $%02X");
    cpu->opcode_table[0x66] = _new_opcode(_addr_ZP, _instr_ROR, 5,"ROR $%02X");
    cpu->opcode_table[0x68] = _new_opcode(_addr_IMP, _instr_PLA, 4,"PLA");
    cpu->opcode_table[0x69] = _new_opcode(_addr_IMM, _instr_ADC, 2,"ADC #$%02X");
    cpu->opcode_table[0x6A] = _new_opcode(_addr_ACC, _instr_ROR, 2,"ROR A");
    cpu->opcode_table[0x6C] = _new_opcode(_addr_IND, _instr_JMP, 5,"JMP ($%04X)");
    cpu->opcode_table[0x6D] = _new_opcode(_addr_ABS, _instr_ADC, 4,"ADC $%04X");
    cpu->opcode_table[0x6E] = _new_opcode(_addr_ABS, _instr_ROR, 6,"ROR $%04X");
    cpu->opcode_table[0x70] = _new_opcode(_addr_REL, _instr_BVS, 2,"BVS $%04X");
    cpu->opcode_table[0x71] = _new_opcode(_addr_IND_Y, _instr_ADC, 5,"ADC ($%04X),Y");
    cpu->opcode_table[0x75] = _new_opcode(_addr_ZP_X, _instr_ADC, 4,"ADC $%02X,X");
    cpu->opcode_table[0x76] = _new_opcode(_addr_ZP_X, _instr_ROR, 6,"ROR $%02X,X");
    cpu->opcode_table[0x78] = _new_opcode(_addr_IMP, _instr_SEI, 2,"SEI");
    cpu->opcode_table[0x79] = _new_opcode(_addr_ABS_Y, _instr_ADC, 4,"ADC $%04X,Y");
    cpu->opcode_table[0x7D] = _new_opcode(_addr_ABS_X, _instr_ADC, 4,"ADC $%04X,X");
    cpu->opcode_table[0x7E] = _new_opcode(_addr_ABS_X, _instr_ROR, 7,"ROR $%04X,X");
    cpu->opcode_table[0x81] = _new_opcode(_addr_IND_X, _instr_STA, 6,"STA ($%04X,X)");
    cpu->opcode_table[0x84] = _new_opcode(_addr_ZP, _instr_STY, 3,"STY $%02X");
    cpu->opcode_table[0x85] = _new_opcode(_addr_ZP, _instr_STA, 3,"STA $%02X");
    cpu->opcode_table[0x86] = _new_opcode(_addr_ZP, _instr_STX, 3,"STX $%02X");
    cpu->opcode_table[0x88] = _new_opcode(_addr_IMP, _instr_DEY, 2,"DEY");
    cpu->opcode_table[0x8A] = _new_opcode(_addr_IMP, _instr_TXA, 2,"TXA");
    cpu->opcode_table[0x8C] = _new_opcode(_addr_ABS, _instr_STY, 4,"STY $%04X");
    cpu->opcode_table[0x8D] = _new_opcode(_addr_ABS, _instr_STA, 4,"STA $%04X");
    cpu->opcode_table[0x8E] = _new_opcode(_addr_ABS, _instr_STX, 4,"STX $%04X");
    cpu->opcode_table[0x90] = _new_opcode(_addr_REL, _instr_BCC, 2,"BCC $%04X");
    cpu->opcode_table[0x91] = _new_opcode(_addr_IND_Y, _instr_STA, 6,"STA ($%04X),Y");
    cpu->opcode_table[0x94] = _new_opcode(_addr_ZP_X, _instr_STY, 4,"STY $%02X,X");
    cpu->opcode_table[0x95] = _new_opcode(_addr_ZP_X, _instr_STA, 4,"STA $%02X,X");
    cpu->opcode_table[0x96] = _new_opcode(_addr_ZP_Y, _instr_STX, 4,"STX $%02X,Y");
    cpu->opcode_table[0x98] = _new_opcode(_addr_IMP, _instr_TYA, 2,"TYA");
    cpu->opcode_table[0x99] = _new_opcode(_addr_ABS_Y, _instr_STA, 5,"STA $%04X,Y");
    cpu->opcode_table[0x9A] = _new_opcode(_addr_IMP, _instr_TXS, 2,"TXS");
    cpu->opcode_table[0x9D] = _new_opcode(_addr_ABS_X, _instr_STA, 5,"STA $%04X,X");
    cpu->opcode_table[0xA0] = _new_opcode(_addr_IMM, _instr_LDY, 2,"LDY #$%02X");
    cpu->opcode_table[0xA1] = _new_opcode(_addr_IND_X, _instr_LDA, 6,"LDA ($%04X,X)");
    cpu->opcode_table[0xA2] = _new_opcode(_addr_IMM, _instr_LDX, 2,"LDX #$%02X");
    cpu->opcode_table[0xA4] = _new_opcode(_addr_ZP, _instr_LDY, 3,"LDY $%02X");
    cpu->opcode_table[0xA5] = _new_opcode(_addr_ZP, _instr_LDA, 3,"LDA $%02X");
    cpu->opcode_table[0xA6] = _new_opcode(_addr_ZP, _instr_LDX, 3,"LDX $%02X");
    cpu->opcode_table[0xA8] = _new_opcode(_addr_IMP, _instr_TAY, 2,"TAY");
    cpu->opcode_table[0xA9] = _new_opcode(_addr_IMM, _instr_LDA, 2,"LDA #$%02X");
    cpu->opcode_table[0xAA] = _new_opcode(_addr_IMP, _instr_TAX, 2,"TAX");
    cpu->opcode_table[0xAC] = _new_opcode(_addr_ABS, _instr_LDY, 4,"LDY $%04X");
    cpu->opcode_table[0xAD] = _new_opcode(_addr_ABS, _instr_LDA, 4,"LDA $%04X");
    cpu->opcode_table[0xAE] = _new_opcode(_addr_ABS, _instr_LDX, 4,"LDX $%04X");
    cpu->opcode_table[0xB0] = _new_opcode(_addr_REL, _instr_BCS, 2,"BCS $%04X");
    cpu->opcode_table[0xB1] = _new_opcode(_addr_IND_Y, _instr_LDA, 5,"LDA ($%04X),Y");
    cpu->opcode_table[0xB4] = _new_opcode(_addr_ZP_X, _instr_LDY, 4,"LDY $%02X,X");
    cpu->opcode_table[0xB5] = _new_opcode(_addr_ZP_X, _instr_LDA, 4,"LDA $%02X,X");
    cpu->opcode_table[0xB6] = _new_opcode(_addr_ZP_Y, _instr_LDX, 4,"LDX $%02X,Y");
    cpu->opcode_table[0xB8] = _new_opcode(_addr_IMP, _instr_CLV, 2,"CLV");
    cpu->opcode_table[0xB9] = _new_opcode(_addr_ABS_Y, _instr_LDA, 4,"LDA $%04X,Y");
    cpu->opcode_table[0xBA] = _new_opcode(_addr_IMP, _instr_TSX, 2,"TSX");
    cpu->opcode_table[0xBC] = _new_opcode(_addr_ABS_X, _instr_LDY, 4,"LDY $%04X,X");
    cpu->opcode_table[0xBD] = _new_opcode(_addr_ABS_X, _instr_LDA, 4,"LDA $%04X,X");
    cpu->opcode_table[0xBE] = _new_opcode(_addr_ABS_Y, _instr_LDX, 4,"LDX $%04X,Y");
    cpu->opcode_table[0xC0] = _new_opcode(_addr_IMM, _instr_CPY, 2,"CPY #$%02X");
    cpu->opcode_table[0xC1] = _new_opcode(_addr_IND_X, _instr_CMP, 6,"CMP ($%04X,X)");
    cpu->opcode_table[0xC4] = _new_opcode(_addr_ZP, _instr_CPY, 3,"CPY $%02X");
    cpu->opcode_table[0xC5] = _new_opcode(_addr_ZP, _instr_CMP, 3,"CMP $%02X");
    cpu->opcode_table[0xC6] = _new_opcode(_addr_ZP, _instr_DEC, 5,"DEC $%02X");
    cpu->opcode_table[0xC8] = _new_opcode(_addr_IMP, _instr_INY, 2,"INY");
    cpu->opcode_table[0xC9] = _new_opcode(_addr_IMM, _instr_CMP, 2,"CMP #$%02X");
    cpu->opcode_table[0xCA] = _new_opcode(_addr_IMP, _instr_DEX, 2,"DEX");
    cpu->opcode_table[0xCC] = _new_opcode(_addr_ABS, _instr_CPY, 4,"CPY $%04X");
    cpu->opcode_table[0xCD] = _new_opcode(_addr_ABS, _instr_CMP, 4,"CMP $%04X");
    cpu->opcode_table[0xCE] = _new_opcode(_addr_ABS, _instr_DEC, 6,"DEC $%04X");
    cpu->opcode_table[0xD0] = _new_opcode(_addr_REL, _instr_BNE, 2,"BNE $%04X");
    cpu->opcode_table[0xD1] = _new_opcode(_addr_IND_Y, _instr_CMP, 5,"CMP ($%04X),Y");
    cpu->opcode_table[0xD5] = _new_opcode(_addr_ZP_X, _instr_CMP, 4,"CMP $%02X,X");
    cpu->opcode_table[0xD6] = _new_opcode(_addr_ZP_X, _instr_DEC, 6,"DEC $%02X,X");
    cpu->opcode_table[0xD8] = _new_opcode(_addr_IMP, _instr_CLD, 2,"CLD");
    cpu->opcode_table[0xD9] = _new_opcode(_addr_ABS_Y, _instr_CMP, 4,"CMP $%04X,Y");
    cpu->opcode_table[0xDD] = _new_opcode(_addr_ABS_X, _instr_CMP, 4,"CMP $%04X,X");
    cpu->opcode_table[0xDE] = _new_opcode(_addr_ABS_X, _instr_DEC, 7,"DEC $%04X,X");
    cpu->opcode_table[0xE0] = _new_opcode(_addr_IMM, _instr_CPX, 2,"CPX #$%02X");
    cpu->opcode_table[0xE1] = _new_opcode(_addr_IND_X, _instr_SBC, 6,"SBC ($%04X,X)");
    cpu->opcode_table[0xE4] = _new_opcode(_addr_ZP, _instr_CPX, 4,"CPX $%02X");
    cpu->opcode_table[0xE5] = _new_opcode(_addr_ZP, _instr_SBC, 3,"SBC $%02X");
    cpu->opcode_table[0xE6] = _new_opcode(_addr_ZP, _instr_INC, 5,"INC $%02X");
    cpu->opcode_table[0xE8] = _new_opcode(_addr_IMP, _instr_INX, 2,"INX");
    cpu->opcode_table[0xE9] = _new_opcode(_addr_IMM, _instr_SBC, 2,"SBC #$%02X");
    cpu->opcode_table[0xEA] = _new_opcode(_addr_IMP, _instr_NOP, 2,"NOP");
    cpu->opcode_table[0xEC] = _new_opcode(_addr_ABS, _instr_CPX, 4,"CPX $%04X");
    cpu->opcode_table[0xED] = _new_opcode(_addr_ABS, _instr_SBC, 4,"SBC $%04X");
    cpu->opcode_table[0xEE] = _new_opcode(_addr_ABS, _instr_INC, 6,"INC $%04X");
    cpu->opcode_table[0xF0] = _new_opcode(_addr_REL, _instr_BEQ, 2,"BEQ $%04X");
    cpu->opcode_table[0xF1] = _new_opcode(_addr_IND_Y, _instr_SBC, 5,"SBC ($%04X),Y");
    cpu->opcode_table[0xF5] = _new_opcode(_addr_ZP_X, _instr_SBC, 4,"SBC $%02X,X");
    cpu->opcode_table[0xF6] = _new_opcode(_addr_ZP_X, _instr_INC, 6,"INC $%02X,X");
    cpu->opcode_table[0xF8] = _new_opcode(_addr_IMP, _instr_SED, 2,"SED");
    cpu->opcode_table[0xF9] = _new_opcode(_addr_ABS_Y, _instr_SBC, 4,"SBC $%04X,Y");
    cpu->opcode_table[0xFD] = _new_opcode(_addr_ABS_X, _instr_SBC, 4,"SBC $%04X,X");
    cpu->opcode_table[0xFE] = _new_opcode(_addr_ABS_X, _instr_INC, 7,"INC $%04X,X");
}

int cpu_init(CPU* cpu, Bus* bus) {
    cpu->A       = 0;
    cpu->X       = 0;
    cpu->Y       = 0;
    cpu->SP      = 0xFD;
    cpu->P.value = 0x24;    // typical 6502 reset flags
    cpu->cycles  = 0;
    cpu->PC = 0xFFFC;
    init_opcode_table(cpu);
    cpu->bus = bus;
    cpu->nmi_pending = false;
    return 0;
}

void cpu_reset(CPU* cpu)
{
    cpu->A       = 0;
    cpu->X       = 0;
    cpu->Y       = 0;
    cpu->SP      = 0xFD;
    cpu->P.value = 0x24;    // typical 6502 reset flags
    cpu->cycles  = 0;
    cpu->PC = 0xFFFC;
    cpu->nmi_pending = false;

    uint16_t reset_address = _addr_ABS(cpu);
    _instr_JMP(cpu, reset_address);
}

int cpu_step(CPU* cpu)
{
    if (cpu->nmi_pending) {
        cpu->nmi_pending = false;
        cpu_handle_nmi(cpu);
        return 0; 
    }

    cpu->cycles = 0;
    uint16_t offset = cpu->PC;
    uint8_t op = _read_PC_incr(cpu);
    OpcodeDefinition opcode = cpu->opcode_table[op];
    uint16_t address = 0;
    if (opcode.addr_func) {
        address = opcode.addr_func(cpu);
    }
    if (opcode.instr_func) {
        opcode.instr_func(cpu, address);
    }
    cpu->cycles += opcode.base_cycles;
    if (opcode.disassembly[4] == '#') {
        uint8_t val = cpu_read(cpu->bus, address);
        snprintf(cpu->disassembly, 16, opcode.disassembly, val);
    }
    else if (opcode.disassembly[12] == 'Y') {
        uint8_t val = cpu_read(cpu->bus, offset + 1);
        snprintf(cpu->disassembly, 16, opcode.disassembly, val);
    }
    else if (op == 0x6C) { 
        uint8_t val_lo = cpu_read(cpu->bus, offset + 1);
        uint8_t val_hi = cpu_read(cpu->bus, offset + 2);
        uint16_t ind_addr = (uint16_t)(val_lo | (val_hi << 8));
        snprintf(cpu->disassembly, 16, opcode.disassembly, ind_addr);
    }
    else {
        snprintf(cpu->disassembly, 16, opcode.disassembly, address);
    }
    return cpu->cycles;
}

void cpu_handle_nmi(CPU* cpu)
{
    _stack_push(cpu, (uint8_t)(cpu->PC >> 8));
    _stack_push(cpu, (uint8_t)(cpu->PC & 0xFF));
    _stack_push(cpu, (uint8_t)cpu->P.value);
    cpu->PC = 0xFFFA;
    uint16_t nmi_address = _addr_ABS(cpu);
    _instr_JMP(cpu, nmi_address);
    cpu->P.I = 1;
}

void _branch(CPU* cpu, int flag, int check, uint16_t address)
{
    if (flag == check) {
        cpu->cycles += 1;
        // If page boundary crossed, add cycle
        if ((cpu->PC >> 8) != (address >> 8)) {
            cpu->cycles += 1;
        }
        cpu->PC = address;
    }
}

uint8_t _read_PC_incr(CPU* cpu)
{
    uint8_t val = cpu_read(cpu->bus, cpu->PC);
    cpu->PC += 1;
    return val;
}

void _stack_push(CPU* cpu, uint8_t value)
{
    // 6502 stack is always page 1 (0x0100 + SP)
    uint16_t stack_addr = 0x0100 | cpu->SP;
    cpu_write(cpu->bus, stack_addr, value);
    cpu->SP = (uint8_t)((cpu->SP - 1) & 0xFF);
}

uint8_t _stack_pull(CPU* cpu)
{
    cpu->SP = (uint8_t)((cpu->SP + 1) & 0xFF);
    uint16_t stack_addr = 0x0100 | cpu->SP;
    return cpu_read(cpu->bus, stack_addr);
}