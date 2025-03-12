#include "cartridge/cartridge.h"
#include "cartridge/mapper/mapper_001.h"


// Initialize the mapper
static void mapper001_init(Cartridge* cart) {
    Mapper001State* state = malloc(sizeof(Mapper001State));
    if(!state) {
        return;
    }
    memset(state, 0, sizeof(Mapper001State));
    state->shift_register = 0x10;
    state->control.prg_bank_mode = 0x03;
    state->prg_bank = 0;
    state->chr_bank_0 = 0;
    state->chr_bank_1 = 0;
    cart->mapper->state = state;
    mapper001_update_banks(cart);
}

// Read PRG-ROM
static uint8_t mapper001_prg_read(Cartridge* cart, uint16_t address) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    uint32_t prg_rom_size = cart->header.prg_rom_size;
    uint32_t bank_address = 0;
    switch (state->control.prg_bank_mode) {
        case 0:
        case 1:  // 32KB mode
            bank_address = ((state->prg_bank & 0x0E) << 14) + address;
        case 2:  // Fixed first bank, switchable upper bank
            bank_address = (address >= 0x4000 ? (state->prg_bank << 14) : 0) + (address & 0x3FFF);
        case 3:  // Switchable lower bank, fixed upper bank
            bank_address = (address < 0x4000 ? (state->prg_bank << 14) : ((cart->header.prg_rom_size - 1) << 14)) + (address & 0x3FFF);
    }
    return cart->prg_rom[bank_address];
}

static void mapper001_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    // Reset shift register on bit 7 set
    if (value & 0x80) {
        state->shift_register = 0x10;
        return;
    }
    // Shift register logic
    uint8_t full = state->shift_register & 0x01;
    state->shift_register >>= 1;
    state->shift_register |= (value & 1) << 4;

    if (full) {
        uint8_t target_register = (address >> 13) & 0x03;
        switch (target_register) {
            case 0: // Control
                state->control.value = state->shift_register & 0x1F;
                break;
            case 1: // CHR Bank 0
                state->chr_bank_0 = state->shift_register;
                break;
            case 2: // CHR Bank 1
                state->chr_bank_1 = state->shift_register;
                break;
            case 3: // PRG Bank
                state->prg_bank = state->shift_register & 0x0F;
                break;
        }
        state->shift_register = 0x10; // Reset shift register
        mapper001_update_banks(cart);
    }
}
static uint8_t mapper001_chr_read(Cartridge* cart, uint16_t address) {
    if (cart->chr_ram) {
        return cart->chr_ram[address & 0x1FFF];
    } else {
        Mapper001State* state = (Mapper001State*)cart->mapper->state;
        if ((state->control.chr_bank_mode) == 0) {  // 8KB CHR mode
            return cart->chr_rom[(state->chr_bank_0 & 0x1E) * 0x1000 + (address & 0x1FFF)];
        } else {  // 4KB CHR mode
            if (address < 0x1000) {
                return cart->chr_rom[state->chr_bank_0 * 0x1000 + (address & 0x0FFF)];
            } else {
                return cart->chr_rom[state->chr_bank_1 * 0x1000 + (address & 0x0FFF)];
            }
        }
    }
}
static void mapper001_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->chr_ram) {
        cart->chr_ram[address & 0x1FFFF] = value;
    }
}
static uint8_t mapper001_wram_read(Cartridge* cart, uint16_t address){
    if(cart->prg_ram) {
        return cart->prg_ram[address & 0x1FFF];
    }
    return 0;
}
static void mapper001_wram_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if(cart->prg_ram) {
        cart->prg_ram[address & 0x1FFF] = value;
    }
}

static void mapper001_update_banks(Cartridge* cart) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
}
static void mapper001_free(Cartridge* cart) {
    if (cart->mapper->state) {
        free(cart->mapper->state);
        cart->mapper->state = NULL;
    }
}

// Instantiate the mapper
Mapper MAPPER_001 = {
    .init = mapper001_init,
    .read_prg = mapper001_prg_read,
    .write_prg = mapper001_prg_write,
    .read_chr = mapper001_chr_read,
    .write_chr = mapper001_chr_write,
    .read_wram = mapper001_wram_read,
    .write_wram = mapper001_wram_write,
    .free = mapper001_free
};