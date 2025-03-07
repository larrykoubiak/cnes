#include "mapper.h"
#include "cartridge.h"

void mapper000_init(Cartridge* cart) {
}

uint8_t mapper000_prg_read(Cartridge* cart, uint16_t address) {
    return cart->prg_rom[address & (cart->prg_rom_size - 1)];
}

void mapper000_prg_write(Cartridge* cart, uint16_t address, uint8_t value) {
}

uint8_t mapper000_chr_read(Cartridge* cart, uint16_t address){
    return cart->chr_rom[address & (cart->chr_rom_size - 1)];
}

void mapper000_chr_write(Cartridge* cart, uint16_t address, uint8_t value) {
}

Mapper MAPPER_000 = {
    .init=mapper000_init,
    .read_prg=mapper000_prg_read,
    .write_prg=mapper000_prg_write,
    .read_chr=mapper000_chr_read,
    .write_chr=mapper000_chr_write
};

Mapper* get_mapper(uint16_t mapper_id){
    if(mapper_id == 0) {
        return &MAPPER_000;
    }
    return NULL;
}
