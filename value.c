#include "value.h"
#include "common.h"
#include "memory.h"
#include <stdio.h>


void initValueArray(ValueArray*array){
    array->capacity = 0;
    array->size = 0;
    array->values = NULL;
}

void writeValueArray(ValueArray*array,Value value){
    if(array->size == array->capacity){
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value,array->values,oldCapacity,array->capacity);
    }
    array->values[array->size] = value;
    array->size++;
}

void freeValueArray(ValueArray *array){
    FREE_ARRAY(Value,array->values,array->capacity);
    initValueArray(array);
}

void printValue(Value value){
    printf("%g",value);
}