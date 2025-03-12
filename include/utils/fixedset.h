#ifndef FIXEDSET_H
#define FIXEDSET_H

#include <stdbool.h>
#include <string.h>

typedef struct FixedIntSet{
    int data[1024];
    int size;
    int capacity;
} FixedIntSet;

void fixedset_insert(FixedIntSet* set, int key);
bool fixedset_contains(FixedIntSet* set, int key);
void fixedset_remove(FixedIntSet* set, int key);

#endif