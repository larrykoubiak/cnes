#include "cartridge/cartridge.h"
#include "cartridge/mapper/mapper.h"

int cart_init(Cartridge* cart, const char* filename) {
    LoadCartridgeResult result = cart_load(cart, filename);
    switch(result) {
        case LOAD_CARTRIDGE_OK:
            return 0;
        case LOAD_CARTRIDGE_FILE_ERROR:
            fprintf(stderr, "Failed to open cartridge file %s\n", filename);
            return 1;
        case LOAD_CARTRIDGE_HEADER_ERROR:
            fprintf(stderr, "Failed to read iNES header in %s\n", filename);
            return 1;
        case LOAD_CARTRIDGE_MEM_ERROR:
            fprintf(stderr, "Failed to allocate memory for cartridge %s\n", filename);
            return 1;
        case LOAD_CARTRIDGE_READ_ERROR:
            fprintf(stderr, "Failed to read from file %s\n", filename);
            return 1;
        default:
            return 1;
    }
}

LoadCartridgeResult cart_load(Cartridge* cart, const char* filename){
    uint8_t header_bytes[16];
    uint32_t prg_rom_size, chr_rom_size, prg_ram_size, chr_ram_size;
    iNESHeader header;
    FILE* f = fopen(filename, "rb");
    if(!f) {
        return LOAD_CARTRIDGE_FILE_ERROR;
    }
    if (fread(header_bytes, 1, 16, f) != 16) {
        fclose(f);
        return LOAD_CARTRIDGE_HEADER_ERROR;
    }
    if (parse_ines_header(header_bytes, &header) != INES_OK) {
        fclose(f);
        return LOAD_CARTRIDGE_HEADER_ERROR;
    }
    cart->header = header;
    prg_rom_size = header.prg_rom_size * 0x4000;
    chr_rom_size = header.chr_rom_size * 0x2000;
    prg_ram_size = header.prg_ram_size;
    chr_ram_size = header.chr_ram_size;
    cart->prg_rom = (uint8_t*)malloc(prg_rom_size);
    if (!cart->prg_rom) {
        fclose(f);
        return LOAD_CARTRIDGE_MEM_ERROR;
    }
    if (fread(cart->prg_rom, 1, prg_rom_size, f) != prg_rom_size) {
        free(cart->prg_rom);
        fclose(f);
        return LOAD_CARTRIDGE_READ_ERROR;
    }
    cart->prg_rom_size = prg_rom_size;
    if (chr_rom_size > 0) {
        cart->chr_rom = (uint8_t*)malloc(chr_rom_size);
        if (!cart->chr_rom) {
            free(cart->prg_rom);
            fclose(f);
            return LOAD_CARTRIDGE_MEM_ERROR;
        }
        if (fread(cart->chr_rom, 1, chr_rom_size, f) != chr_rom_size) {
            free(cart->prg_rom);
            free(cart->chr_rom);
            fclose(f);
            return LOAD_CARTRIDGE_READ_ERROR;
        }
        cart->chr_rom_size = chr_rom_size;
    } else {
        cart->chr_rom = NULL;
        cart->chr_rom_size = 0;
    }
    fclose(f);
    // RAM initialization
    if(prg_ram_size > 0) {
        cart->prg_ram = (uint8_t*)malloc(prg_ram_size);
        if (!cart->prg_ram) {
            cart_free(cart);
            return LOAD_CARTRIDGE_MEM_ERROR;
        }
        memset(cart->prg_ram, 0, prg_ram_size);
        cart->prg_ram_size = prg_ram_size;
    } else {
        cart->prg_ram = NULL;
        cart->prg_ram_size = 0;
    }
    if (chr_ram_size > 0) {
        cart->chr_ram = (uint8_t*) malloc(chr_ram_size);
        if (!cart->chr_ram) {
            cart_free(cart);
            return LOAD_CARTRIDGE_MEM_ERROR;
        }
        memset(cart->chr_ram, 0, chr_ram_size);
        cart->chr_ram_size = chr_ram_size;
    } else if (cart->chr_rom_size == 0) { //homebrew needs this
        chr_ram_size = 0x2000;
        cart->chr_ram = (uint8_t*) malloc(chr_ram_size);
        if (!cart->chr_ram) {
            cart_free(cart);
            return LOAD_CARTRIDGE_MEM_ERROR;
        }
        memset(cart->chr_ram, 0, chr_ram_size);
        cart->chr_ram_size = chr_ram_size;
    } else {
        cart->chr_ram = NULL;
        cart->chr_ram_size = 0;
    }
    cart->mapper = get_mapper(header.mapper);
    cart->mapper->init(cart);
    return LOAD_CARTRIDGE_OK;
}

void cart_free(Cartridge* cart) {
    if (cart->mapper && cart->mapper->free) {
        
    }
    if (cart->prg_rom) {
        free(cart->prg_rom);
    }
    if (cart->chr_rom) {
        free(cart->chr_rom);
    }
    if (cart->prg_ram) {
        free(cart->prg_ram);
    }
    if (cart->chr_ram) {
        free(cart->chr_ram);
    }
}