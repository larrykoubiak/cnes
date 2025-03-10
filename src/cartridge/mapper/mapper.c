#include "mapper.h"
#include "mapper_000.h"
#include "mapper_001.h"
#include "mapper_002.h"

Mapper* get_mapper(uint16_t mapper_id){
    switch(mapper_id) {
        case 0: return &MAPPER_000;
        case 1: return &MAPPER_001;
        case 2: return &MAPPER_002;
        default: return NULL;
    }
    return NULL;
}
