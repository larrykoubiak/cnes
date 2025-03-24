#include "cartridge/mapper/mapper.h"
#include "cartridge/mapper/mapper_000.h"
#include "cartridge/mapper/mapper_001.h"
#include "cartridge/mapper/mapper_002.h"
#include "cartridge/mapper/mapper_003.h"

Mapper* get_mapper(uint16_t mapper_id){
    switch(mapper_id) {
        case 0: return &MAPPER_000;
        case 1: return &MAPPER_001;
        case 2: return &MAPPER_002;
        case 3: return &MAPPER_003;
        default: return NULL;
    }
    return NULL;
}
