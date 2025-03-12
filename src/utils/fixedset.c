#include "utils/fixedset.h"

void fixedset_insert(FixedIntSet* set, int key){
    int i;
    if (!set){
        return;
    }
    for (i=0;i < set->size;i++){
        if(set->data[i] == key){
            return;
        }
    }
    if (set->size < 1024){
        set->data[set->size] = key;
        set->size += 1;
    }
}

bool fixedset_contains(FixedIntSet* set, int key){
    int i;
    if (!set){
        return;
    }
    for (i=0;i < set->size;i++){
        if(set->data[i] == key){
            return true;
        }
    }
    return false;
}

void fixedset_remove(FixedIntSet* set, int key){
    int i;
    if (!set){
        return;
    }
    for (i=0;i < set->size;i++){
        if(set->data[i] == key){
            set->data[i] = set->data[set->size - 1];
            set->size -= 1;
            return;
        }
    }
}