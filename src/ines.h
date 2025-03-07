#ifndef INES_H
#define INES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    VERTICAL_LAYOUT = 0,
    HORIZONTAL_LAYOUT = 1
} NametableLayout;

typedef enum {
    NES_FAMICOM = 0,
    VS_SYSTEM = 1,
    PLAYCHOICE_1O = 2,
    FAMICLONE_BCD = 3,
    FAMICOM_ESPM = 4,
    VT01 = 5,
    VT02 = 6,
    VT03 = 7,
    VT09 = 8,
    VT32 = 9,
    VT369 = 10,
    UM6578 = 11,
    FAMICOM_NETWORK_SYSTEM = 12
} ConsoleType;

typedef enum {
    RP2C02 = 0,
    RP2C07 = 1,
    MULTI_REGION = 2,
    UA6538 = 3
} TimingMode;

typedef enum {
    NO_RAM = 0,
    NVRAM = 1,
    VRAM = 2
} RAMType;

typedef enum {
    RP2C03_RC2C03 = 0,
    RP2C04_0001 = 2,
    RP2C04_0002 = 3,
    RP2C04_0003 = 4,
    RP2C04_0004 = 5,
    RC2C05_01 = 8,
    RC2C05_02 = 9,
    RC2C05_03 = 10,
    RC2C05_04 = 11,
    VS_NONE = 255
} VSSystem;

typedef enum{
    VS_UNISYSTEM_NORMAL = 0,
    VS_UNISYSTEM_RBI_BASEBALL = 1,
    VS_UNISYSTEM_TKO_BOXING = 2,
    VS_UNISYSTEM_SUPER_XEVIOUS = 3,
    VS_UNISYSTEM_VS_ICE_CLIMBER_JAPAN = 4,
    VS_DUAL_SYSTEM_NORMAL = 5,
    VS_DUAL_SYSTEM_RAID_ON_BUNGELING_BAY = 6,
    VS_PPU_NONE = 255
} VSPPU;

typedef enum {
    UNSPECIFIED = 0,
    STANDARD_NES_FAMICOM_CONTROLLERS = 1,
    NES_FOUR_SCORE_SATELLITE = 2,
    FAMICOM_FOUR_PLAYERS_ADAPTER = 3,
    VS_SYSTEM_1P_VIA_4016 = 4,
    VS_SYSTEM_1P_VIA_4017 = 5,
    RESERVED = 6,
    VS_ZAPPER = 7,
    ZAPPER_4017 = 8,
    TWO_ZAPPERS = 9,
    BANDAI_HYPER_SHOT_LIGHTGUN = 10,
    POWER_PAD_SIDE_A = 11,
    POWER_PAD_SIDE_B = 12,
    FAMILY_TRAINER_SIDE_A = 13,
    FAMILY_TRAINER_SIDE_B = 14,
    ARKANOID_VAUS_CONTROLLER_NES = 15,
    ARKANOID_VAUS_CONTROLLER_FAMICOM = 16,
    TWO_VAUS_CONTROLLERS_PLUS_FAMICOM_DATA_RECORDER = 17,
    KONAMI_HYPER_SHOT_CONTROLLER = 18,
    COCONUTS_PACHINKO_CONTROLLER = 19,
    EXCITING_BOXING_PUNCHING_BAG = 20,
    JISSEN_MAHJONG_CONTROLLER = 21,
    PARTY_TAP = 22,
    OEKA_KIDS_TABLET = 23,
    SUNSOFT_BARCODE_BATTLER = 24,
    MIRACLE_PIANO_KEYBOARD = 25,
    POKKUN_MOGURAA = 26,
    TOP_RIDER = 27,
    DOUBLE_FISTED = 28,
    FAMICOM_3D_SYSTEM = 29,
    DOREMIKKO_KEYBOARD = 30,
    ROB_GYRO_SET = 31,
    FAMICOM_DATA_RECORDER = 32,
    ASCII_TURBO_FILE = 33,
    IGS_STORAGE_BATTLE_BOX = 34,
    FAMILY_BASIC_KEYBOARD_PLUS_FAMICOM_DATA_RECORDER = 35,
    DONGDA_PEC_KEYBOARD = 36,
    BIT_79_KEYBOARD = 37,
    SUBOR_KEYBOARD = 38,
    SUBOR_KEYBOARD_PLUS_MOUSE_3X8_BIT_PROTOCOL = 39,
    SUBOR_KEYBOARD_PLUS_MOUSE_24_BIT_PROTOCOL_4016 = 40,
    SNES_MOUSE = 41,
    MULTICART = 42,
    TWO_SNES_CONTROLLERS = 43,
    RACERMATE_BICYCLE = 44,
    U_FORCE = 45,
    ROB_STACK_UP = 46,
    CITY_PATROLMAN_LIGHTGUN = 47,
    SHARP_C1_CASSETTE_INTERFACE = 48,
    STANDARD_CONTROLLER_SWAPPED = 49,
    EXCALIBUR_SUDOKU_PAD = 50,
    ABL_PINBALL = 51,
    GOLDEN_NUGGET_CASINO_EXTRA_BUTTONS = 52,
    KEDA_KEYBOARD = 53,
    SUBOR_KEYBOARD_PLUS_MOUSE_24_BIT_PROTOCOL_4017 = 54,
    PORT_TEST_CONTROLLER = 55,
    BANDAI_MULTI_GAME_PLAYER_GAMEPAD_BUTTONS = 56,
    VENOM_TV_DANCE_MAT = 57,
    LG_TV_REMOTE_CONTROL = 58,
    FAMICOM_NETWORK_CONTROLLER = 59,
    KING_FISHING_CONTROLLER = 60,
    CROAKY_KARAOKE_CONTROLLER = 61,
    KINGWON_KEYBOARD = 62,
    ZECHENG_KEYBOARD = 63,
    SUBOR_KEYBOARD_PLUS_PS2_MOUSE_AND_ADAPTER_4017 = 64
} ExpansionDevice;

typedef enum{
    INES_OK = 0,
    INES_INVALID_MAGIC = 1,
    INES_UNSUPPORTED_VERSION = 2
} iNESError;

typedef struct iNESHeader {
    uint16_t prg_rom_size;
    uint16_t chr_rom_size;
    uint8_t ines_version;
    ConsoleType console_type;
    NametableLayout nametable_layout;
    bool alternate_layout;
    bool trainer_at_512;
    bool has_battery;
    uint16_t mapper;
    uint8_t submapper;
    RAMType prg_ram_type;
    RAMType chr_ram_type;
    uint32_t prg_ram_size;
    uint32_t chr_ram_size;
    TimingMode timing_mode;
    uint8_t miscellaneous_roms;
    ExpansionDevice default_expansion_device;
    VSSystem vs_system;
    VSPPU vs_ppu;
} iNESHeader;

iNESError parse_ines_header(uint8_t* data, iNESHeader* header);

#endif