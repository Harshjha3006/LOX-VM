#include "value.h"
#include "common.h"
#include "memory.h"
#include "object.h"
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
    #ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "true" : "false");
    } 
    else if (IS_NIL(value)) {
        printf("nil");
    } 
    else if (IS_NUM(value)) {
        printf("%g", AS_NUM(value));
    } 
    else if (IS_OBJ(value)) {
        printObject(value);
    }

    #else
    switch(value.type){
        case VAL_NUM:
            printf("%g",value.as.number);
            break;
        case VAL_BOOL:
            printf(value.as.boolean?"true":"false");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_OBJ:
            printObject(value);
            break;
        default:
            return;
    }

    #endif
}