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
#define IS_CLASS(value) isObjType(value,OBJ_CLASS)
#define IS_INSTANCE(value) isObjType(value,OBJ_INSTANCE)
#define IS_BOUND_METHOD(value) isObjType(value,OBJ_BOUND_METHOD)


typedef enum{
    OBJ_STR,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
}ObjType;

struct Obj{
    ObjType type;
    Obj*next;
    bool isMarked;
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
    ObjString*name; // name of functionn
    Chunk chunk; // function's chunk to which its bytecode will be emitted
    int arity; // no of arguements of the function
};

struct ObjClass{
    Obj obj;
    ObjString*name;
    Table methods;
};

struct ObjInstance{
    Obj obj;
    ObjClass*klass;
    Table fields;
};

struct ObjBoundMethod{
    Obj obj;
    ObjFunction*method;
    Value receiver;
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
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))


ObjString* copyString(const char*chars,int length);
ObjString *allocateString(char *chars,int length,uint32_t hash);
uint32_t hashString(const char*key,int length);
ObjString* tableFindString(Table*table,const char*chars,int length,uint32_t hash);
ObjFunction* newFunction();
ObjNative* newNative(NativeFn fn);
ObjClass* newClass(ObjString*name);
ObjInstance *newInstance(ObjClass*klass);
ObjBoundMethod* newBoundMethod(ObjFunction*fn,Value receiver);

static inline bool isObjType(Value value,ObjType type){
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void printObject(Value value);

#endif 