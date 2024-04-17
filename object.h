#ifndef object_h
#define object_h 
#include "value.h"
#include "memory.h"
#include "table.h"


#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value)  isObjType(value,OBJ_STR)



typedef enum{
    OBJ_STR
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

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)


ObjString* copyString(const char*chars,int length);
ObjString *allocateString(char *chars,int length,uint32_t hash);
uint32_t hashString(const char*key,int length);
ObjString* tableFindString(Table*table,const char*chars,int length,uint32_t hash);

static inline bool isObjType(Value value,ObjType type){
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void printObject(Value value);

#endif 