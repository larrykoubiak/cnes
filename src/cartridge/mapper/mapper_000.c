#include "cartridge/cartridge.h"
#include "cartridge/mapper/mapper_000.h"

void mapper000_init(Cartridge* cart) {
}

uint8_t mapper000_prg_read(Cartridge* cart, uint16_t address) {
    return cart->prg_rom[address & (cart->prg_rom_size - 1)];
}

void mapper000_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
}

uint8_t mapper000_chr_read(Cartridge* cart, uint16_t address){
    if(cart->chr_ram) {
        return cart->chr_ram[address & (cart->chr_ram_size - 1)];
    }
    return cart->chr_rom[address & (cart->chr_rom_size - 1)];
}

void mapper000_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if (cart->chr_ram) {
        cart->chr_ram[address & (cart->chr_ram_size - 1)] = value;
    }
}

uint8_t mapper000_wram_read(Cartridge* cart, uint16_t address){
    if(cart->prg_ram) {
        return cart->prg_ram[address & 0x1FFF];
    }
    return 0;
}

void mapper000_wram_write(Cartridge* cart, uint16_t address, uint8_t value) {
    if(cart->prg_ram) {
        cart->prg_ram[address & 0x1FFF] = value;
    }
}

Mapper MAPPER_000 = {
    .init=mapper000_init,
    .read_prg=mapper000_prg_read,
    .write_prg=mapper000_prg_write,
    .read_chr=mapper000_chr_read,
    .write_chr=mapper000_chr_write,
    .read_wram=mapper000_wram_read,
    .write_wram=mapper000_wram_write,
    .free=NULL,
    .state=NULL
};