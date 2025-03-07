#include <stdint.h>
#include <string.h>
#include "ines.h"

iNESError parse_ines_header(uint8_t* data, iNESHeader* header){
    uint8_t prg_rom_lsb, chr_rom_lsb, prg_rom_msb, chr_rom_msb, prg_nvram, prg_ram, chr_nvram, chr_ram;
    uint8_t nametable, alternate, trainer, battery, mapper_lsb, mapper_hsb, mapper_msb, submapper;
    uint8_t ines_version, console_type, timing_mode, system_type_lsb, system_type_msb, miscellaneous_roms, expansion_device;
    
    if(memcmp(data, "NES\x1A", 4) != 0) {
        return INES_INVALID_MAGIC;
    }
    prg_rom_lsb  = data[4];
    chr_rom_lsb = data[5];
    nametable = data[6] & 0x01;
    alternate = (data[6] & 0x02) != 0;
    trainer = (data[6] & 0x04) != 0;
    battery = (data[6] & 0x08) != 0;
    mapper_lsb = data[6] >> 4;
    console_type = data[7] & 0x03;
    ines_version = (data[7] >> 2) & 0x03;
    mapper_hsb = data[7] >> 4;
    mapper_msb = data[8] & 0x0F;
    submapper = (data[8] >> 4) & 0x0F;
    prg_rom_msb = data[9] & 0x0F;
    chr_rom_msb = (data[9] >> 4) & 0x0F;
    prg_ram = data[10] & 0x0F;
    prg_nvram = (data[10] >> 4) & 0x0F;
    chr_ram = data[11] & 0x0F;
    chr_nvram = (data[11] >> 4) & 0x0F;
    timing_mode = data[12] & 0x03;
    system_type_lsb = data[13] & 0x0F;
    system_type_msb = (data[13] >> 4) & 0x0F;
    miscellaneous_roms = data[14] & 0x03;
    expansion_device = data[15] & 0x3F;
    header->prg_rom_size = (prg_rom_msb << 8) | prg_rom_lsb;
    header->chr_rom_size = (chr_rom_msb << 8) | chr_rom_lsb;
    header->nametable_layout = (NametableLayout)nametable;
    header->alternate_layout = alternate;
    header->trainer_at_512 = trainer;
    header->has_battery = battery;
    header->mapper = (mapper_msb << 8) |(mapper_hsb << 4) | mapper_lsb;
    header->submapper = submapper;
    header->console_type = (ConsoleType)console_type;
    header->ines_version = ines_version;
    if (prg_nvram > 0) {
        header->prg_ram_type = NVRAM;
        header->prg_ram_size = 64 << prg_nvram;
    } else if (prg_ram > 0) {
        header->prg_ram_type = VRAM;
        header->prg_ram_size = 64 << prg_ram;
    } else {
        header->prg_ram_type = NO_RAM;
        header->prg_ram_size = 0;
    }
    if (chr_nvram > 0) {
        header->chr_ram_type = NVRAM;
        header->chr_ram_size = 64 << chr_nvram;
    } else if (chr_ram > 0) {
        header->chr_ram_type = VRAM;
        header->chr_ram_size = 64 << chr_ram;
    } else {
        header->chr_ram_type = NO_RAM;
        header->chr_ram_size = 0;
    }
    header->timing_mode = (TimingMode)timing_mode;
    header->vs_system = VS_NONE;
    header->vs_ppu = VS_PPU_NONE;
    if (header->console_type == VS_SYSTEM) {
        header->vs_system = (VSSystem)system_type_lsb;
        header->vs_ppu = (VSPPU)system_type_msb;
    } else if (header->console_type == FAMICLONE_BCD) {
        header->console_type = (ConsoleType)system_type_lsb;
    }     
    header->miscellaneous_roms = miscellaneous_roms;
    header->default_expansion_device = (ExpansionDevice)expansion_device;
    return INES_OK;
};