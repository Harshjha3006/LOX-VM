#ifndef object_h
#define object_h 
#include "value.h"
#include "memory.h"
#include "table.h"
#include "chunk.h"


#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value)  isObjType(value,OBJ_STR)
#define IS_FUNCTION(value) isObjType(value,OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value,OBJ_NATIVE)


typedef enum{
    OBJ_STR,
    OBJ_FUNCTION,
    OBJ_NATIVE
}ObjType;

struct Obj{
    ObjType type;
    Obj*next;
};


struct ObjString{
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

// function object struct 
struct ObjFunction{
    Obj obj; // for inheritance from object struct
    const char*name; // name of functionn
    Chunk chunk; // function's chunk to which its bytecode will be emitted
    int arity; // no of arguements of the function
};

typedef Value (*NativeFn)(int argCount,Value*args);

typedef struct{
    Obj obj;
    NativeFn fn;
}ObjNative;

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE_FN(value) (((ObjNative*)AS_OBJ(value))->fn)


ObjString* copyString(const char*chars,int length);
ObjString *allocateString(char *chars,int length,uint32_t hash);
uint32_t hashString(const char*key,int length);
ObjString* tableFindString(Table*table,const char*chars,int length,uint32_t hash);
ObjFunction* newFunction();
ObjNative* newNative(NativeFn fn);

static inline bool isObjType(Value value,ObjType type){
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void printObject(Value value);

#endif 