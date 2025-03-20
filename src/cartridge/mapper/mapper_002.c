#include "cartridge/cartridge.h"
#include "cartridge/mapper/mapper_002.h"

#define PRG_BANK (cart->mapper->storage[0])

static void mapper002_init(Cartridge* cart) {
    PRG_BANK = 0;
}

static uint8_t mapper002_prg_read(Cartridge* cart, uint16_t address) {
    uint32_t bank_address = 0;
    bank_address = (address < 0x4000 ? (PRG_BANK << 14) : ((cart->header.prg_rom_size - 1) << 14)) + (address & 0x3FFF);
    return cart->prg_rom[bank_address];
}

static void mapper002_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
    PRG_BANK = value & 0x0F;
}

static uint8_t mapper002_chr_read(Cartridge* cart, uint16_t address) {
    if (cart->chr_ram) {
        return cart->chr_ram[address & 0x1FFF];
    } else {
        return cart->chr_rom[address & (cart->chr_rom_size - 1)];
    }
}

static void mapper002_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->chr_ram) {
        cart->chr_ram[address & 0x1FFF] = value;
    }
}

static uint8_t mapper002_wram_read(Cartridge* cart, uint16_t address){
    if(cart->prg_ram) {
        return cart->prg_ram[address & 0x1FFF];
    }
    return 0;
}
static void mapper002_wram_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if(cart->prg_ram) {
        cart->prg_ram[address & 0x1FFF] = value;
    }
}

MirroringMode mapper002_get_mirroring(Cartridge* cart) {
    if (cart->header.alternate_layout) {
        return FOUR_SCREEN;
    }
    switch(cart->header.nametable_layout) {
        case HORIZONTAL_LAYOUT:
            return VERTICAL;
        case VERTICAL_LAYOUT:
            return HORIZONTAL;
        default:
            return HORIZONTAL;
    }
}

static void mapper002_free(Cartridge* cart) {
    if (cart->chr_ram) {
        free(cart->chr_ram);
        cart->chr_ram = NULL;
    }
}

Mapper MAPPER_002 = {
    .init = mapper002_init,
    .read_prg = mapper002_prg_read,
    .write_prg = mapper002_prg_write,
    .read_chr = mapper002_chr_read,
    .write_chr = mapper002_chr_write,
    .read_wram = mapper002_wram_read,
    .write_wram = mapper002_wram_write,
    .get_mirroring = mapper002_get_mirroring,
    .free = mapper002_free,
    .storage = {0,0,0,0,0,0,0,0}
};