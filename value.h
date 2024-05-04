#ifndef value_h
#define value_h
#include "common.h"
#include <string.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjFunction ObjFunction;
typedef struct ObjClass ObjClass;
typedef struct ObjInstance ObjInstance;
typedef struct ObjBoundMethod ObjBoundMethod;

#ifdef NAN_BOXING

typedef uint64_t Value;

#define QNAN     ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define TAG_NIL 1 
#define TAG_FALSE 2
#define TAG_TRUE 3

static Value numToValue(double num){
    Value value;
    memcpy(&value,&num,sizeof(double));
    return value;
}

static double valueToNum(Value value){
    double num;
    memcpy(&num,&value,sizeof(Value));
    return num;
}

#define NUM_VAL(val) numToValue(val)
#define AS_NUM(val) valueToNum(val)
#define IS_NUM(val)  (((val) & QNAN) != QNAN) 

#define NIL_VAL  ((Value)(uint64_t)(TAG_NIL | QNAN))
#define IS_NIL(val) (val == NIL_VAL)

#define FALSE_VAL ((Value)(uint64_t)(TAG_FALSE | QNAN))
#define TRUE_VAL ((Value)(uint64_t)(TAG_TRUE | QNAN))
#define BOOL_VAL(val) ((val) ? TRUE_VAL : FALSE_VAL)
#define AS_BOOL(val) ((val) == TRUE_VAL)
#define IS_BOOL(val) (((val) | 1) == TRUE_VAL)

#define OBJ_VAL(obj) ((SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj)))
#define AS_OBJ(val)  ((Obj*)(uintptr_t)((val) & ~(SIGN_BIT | QNAN)))
#define IS_OBJ(val)  (((val) & (SIGN_BIT | QNAN)) == (SIGN_BIT | QNAN))

#else

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

#endif

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