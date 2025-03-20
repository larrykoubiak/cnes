#include "cartridge/cartridge.h"
#include "cartridge/mapper/mapper_001.h"


// Initialize the mapper
static void mapper001_init(Cartridge* cart) {
    Mapper001State* state = malloc(sizeof(Mapper001State));
    if(!state) {
        return;
    }
    memset(state, 0, sizeof(Mapper001State));
    state->shift_register = 0x0;
    state->control.prg_bank_mode = 0x03;
    state->prg_bank_0 = 0;
    state->prg_bank_1 = (cart->header.prg_rom_size - 1);
    state->chr_bank_0 = 0;
    state->chr_bank_1 = 1;
    state->write_counter = 0;
    cart->mapper->state = state;
}

// Read PRG-ROM
static uint8_t mapper001_prg_read(Cartridge* cart, uint16_t address) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    uint32_t bank_address = ((address & 0x4000) ? state->prg_bank_1 : state->prg_bank_0);
    uint32_t rom_address =  (bank_address << 14) + (address & 0x3FFF);
    return cart->prg_rom[rom_address];
}

static void mapper001_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    // Reset shift register on bit 7 set
    if (value & 0x80) {
        state->shift_register = 0x10;
        state->write_counter = 0;
        return;
    }
    // Shift register logic
    state->shift_register >>= 1;
    state->shift_register |= (value & 1) << 4;
    state->write_counter++;

    if (state->write_counter==5) {
        uint8_t target_register = (address >> 13) & 0x03;
        mapper001_update_banks(cart, target_register);
    }
}
static uint8_t mapper001_chr_read(Cartridge* cart, uint16_t address) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    uint32_t bank_address = ((address & 0x1000) ? state->chr_bank_1 : state->chr_bank_0);
    uint32_t chr_address =  (bank_address << 12) + (address & 0xFFF);
    if (cart->chr_ram) {
        return cart->chr_ram[chr_address];
    } else {
        return cart->chr_rom[chr_address];
    }
}
static void mapper001_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->chr_ram) {
        cart->chr_ram[address & 0x1FFF] = value;
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

static void mapper001_update_banks(Cartridge* cart, uint8_t target_register) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    switch (target_register) {
        case 0: // Control
            state->control.value = state->shift_register & 0x1F;
            break;
        case 1: // CHR Bank 0
            if(state->control.chr_bank_mode == 0) {
                state->chr_bank_0 = state->shift_register & 0x1E;
                state->chr_bank_1 = (state->shift_register & 0x1E) + 1;
            } else {
                state->chr_bank_0 = state->shift_register & 0x1F;
            }
            break;
        case 2: // CHR Bank 1
            if(state->control.chr_bank_mode == 1) {
                state->chr_bank_1 = state->shift_register & 0x1F;
            }
            break;
        case 3: // PRG Bank
            switch(state->control.prg_bank_mode) {
                case 0:
                case 1:
                    state->prg_bank_0 = state->shift_register & 0x0E;
                    state->prg_bank_1 = (state->shift_register & 0x0E) + 1;
                    break;
                case 2:
                    state->prg_bank_0 = 0;
                    state->prg_bank_1 = state->shift_register & 0x0F;
                    break;
                case 3:
                    state->prg_bank_0 = state->shift_register & 0x0F;
                    state->prg_bank_1 = (cart->header.prg_rom_size - 1);
                    break;
                }
            break;
    }
    state->shift_register = 0x0; // Reset shift register
    state->write_counter = 0;
}

MirroringMode mapper001_get_mirroring(Cartridge* cart) {
    Mapper001State* state = (Mapper001State*)cart->mapper->state;
    switch(state->control.mirroring) {
        case 0:
            return SINGLE_SCREEN_LOWER;
        case 1:
            return SINGLE_SCREEN_UPPER;
        case 2:
            return VERTICAL;
        case 3:
            return HORIZONTAL;
    }
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
    .get_mirroring = mapper001_get_mirroring,
    .free = mapper001_free
};