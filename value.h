#ifndef value_h
#define value_h
#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjFunction ObjFunction;
typedef struct ObjClass ObjClass;
typedef struct ObjInstance ObjInstance;
typedef struct ObjBoundMethod ObjBoundMethod;

typedef enum{
    VAL_NUM,
    VAL_BOOL,
    VAL_NIL,
    VAL_OBJ,
}ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        bool boolean;
        Obj* obj;
    }as;
}Value;

#define NUM_VAL(val) ((Value){VAL_NUM,{.number = val}})
#define BOOL_VAL(val) ((Value){VAL_BOOL,{.boolean = val}})
#define NIL_VAL  ((Value){VAL_NIL,{.number = 0}})
#define OBJ_VAL(val) ((Value){VAL_OBJ,{.obj = (Obj*)(val)}})

#define AS_NUM(val) ((val).as.number)
#define AS_BOOL(val) ((val).as.boolean)
#define AS_OBJ(val) ((Obj*)((val).as.obj))

#define IS_BOOL(val) ((val).type == VAL_BOOL)
#define IS_NUM(val) ((val).type == VAL_NUM)
#define IS_NIL(val) ((val).type == VAL_NIL)
#define IS_OBJ(val) ((val).type == VAL_OBJ)

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