#include "cartridge/cartridge.h"
#include "cartridge/mapper/mapper_003.h"

#define CHR_BANK (cart->mapper->storage[0])

static void mapper003_init(Cartridge* cart) {
    CHR_BANK = 0;
}

static uint8_t mapper003_prg_read(Cartridge* cart, uint16_t address) {
    return cart->prg_rom[address & 0x7FFF];
}

static void mapper003_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
    CHR_BANK = value & 0x0F;
}

static uint8_t mapper003_chr_read(Cartridge* cart, uint16_t address) {
    if (cart->chr_ram) {
        return cart->chr_ram[address & 0x1FFF];
    } else {
        uint32_t bank_address = (CHR_BANK << 13) + (address & 0x1FFF);
        return cart->chr_rom[bank_address];
    }
}

static void mapper003_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->chr_ram) {
        cart->chr_ram[address & 0x1FFF] = value;
    }
}

static uint8_t mapper003_wram_read(Cartridge* cart, uint16_t address){
    if (cart->prg_ram) {
        return cart->prg_ram[address & 0x1FFF];
    }
    return 0;
}

static void mapper003_wram_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->prg_ram) {
        cart->prg_ram[address & 0x7FF] = value;
    }
}

MirroringMode mapper003_get_mirroring(Cartridge* cart) {
    if (cart->header.alternate_layout) {
        return FOUR_SCREEN;
    }
    switch (cart->header.nametable_layout) {
        case HORIZONTAL_LAYOUT:
            return VERTICAL;
        case VERTICAL_LAYOUT:
            return HORIZONTAL;
        default:
            return HORIZONTAL;
    }
}

static void mapper003_free(Cartridge* cart) {
    if (cart->chr_ram) {
        free(cart->chr_ram);
        cart->chr_ram = NULL;
    }
}

Mapper MAPPER_003 = {
    .init         = mapper003_init,
    .read_prg     = mapper003_prg_read,
    .write_prg    = mapper003_prg_write,
    .read_chr     = mapper003_chr_read,
    .write_chr    = mapper003_chr_write,
    .read_wram    = mapper003_wram_read,
    .write_wram   = mapper003_wram_write,
    .get_mirroring= mapper003_get_mirroring,
    .free         = mapper003_free,
    .storage      = {0,0,0,0,0,0,0,0}
};
