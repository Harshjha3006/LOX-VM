#ifndef value_h
#define value_h
#include "common.h"


typedef enum{
    VAL_NUM,
    VAL_BOOL,
    VAL_NIL
}ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        bool boolean;
    }as;
}Value;

#define NUM_VAL(val) ((Value){VAL_NUM,{.number = val}})
#define BOOL_VAL(val) ((Value){VAL_BOOL,{.boolean = val}})
#define NIL_VAL  ((Value){.type = VAL_NIL,{.number = 0}})

#define AS_NUM(val) ((val).as.number)
#define AS_BOOL(val) ((val).as.boolean)

#define IS_BOOL(val) ((val).type == VAL_BOOL)
#define IS_NUM(val) ((val).type == VAL_NUM)
#define IS_NIL(val) ((val).type == VAL_NIL)

// struct for dynamic array of constants in the program
typedef struct{
    Value*values;
    int capacity;
    int size;
}ValueArray;

// initialises the value array
void initValueArray(ValueArray*array);
// appending a value in the array
void writeValueArray(ValueArray *array,Value value);
// freeing the value array
void freeValueArray(ValueArray *array);
// printing the value
void printValue(Value value);


#endif