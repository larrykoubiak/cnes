#include "../cartridge.h"
#include "mapper_002.h"

#define PRG_BANK (cart->mapper->storage[0])

// **Initialize Mapper 002**
static void mapper002_init(Cartridge* cart) {
    // Start with PRG bank 0 selected
    PRG_BANK = 0;
    if (cart->chr_rom_size == 0) {
        cart->chr_ram = calloc(0x2000, sizeof(uint8_t)); // 8KB CHR-RAM
    }
}

// **PRG-ROM Read**
static uint8_t mapper002_prg_read(Cartridge* cart, uint16_t address) {
    uint32_t bank_address = 0;
    bank_address = (address < 0x4000 ? (PRG_BANK << 14) : ((cart->header.prg_rom_size - 1) << 14)) + (address & 0x3FFF);
    return cart->prg_rom[bank_address];
}

// **PRG-ROM Write (Switch PRG Bank)**
static void mapper002_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
    PRG_BANK = value & 0x0F;
}

// **CHR-ROM Read (Always Fixed)**
static uint8_t mapper002_chr_read(Cartridge* cart, uint16_t address) {
    if (cart->chr_ram) {
        return cart->chr_ram[address & 0x1FFF]; // 8KB CHR-RAM
    } else {
        return cart->chr_rom[address & (cart->chr_rom_size - 1)];
    }
}

// **CHR-ROM Write (No Effect, since CHR is ROM)**
static void mapper002_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->chr_ram) {
        cart->chr_ram[address & 0x1FFF] = value; // Write to CHR-RAM
    }
}

static void mapper002_free(Cartridge* cart) {
    if (cart->chr_ram) {
        free(cart->chr_ram);
        cart->chr_ram = NULL;
    }
}

// Instantiate Mapper 002
Mapper MAPPER_002 = {
    .init = mapper002_init,
    .read_prg = mapper002_prg_read,
    .write_prg = mapper002_prg_write,
    .read_chr = mapper002_chr_read,
    .write_chr = mapper002_chr_write,
    .free = mapper002_free,
    .storage = {0}
};