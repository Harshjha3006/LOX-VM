#include "object.h"
#include "vm.h"
#include <string.h>
#include <stdio.h>


static Obj* allocateObject(size_t size,ObjType type){
    Obj *obj = (Obj*)reallocate(NULL,0,size);
    obj->type = type;
    obj->next = vm.objects;
    vm.objects = obj;
    return obj;
}

#define ALLOCATE_OBJ(type,objectType) (type*)allocateObject(sizeof(type),objectType)


ObjString *allocateString(char *chars,int length){
    ObjString*string = ALLOCATE_OBJ(ObjString,OBJ_STR);
    string->chars = chars;
    string->length = length;
    return string;
}

ObjString* copyString(const char *chars,int length){
    char *heapChars = ALLOCATE(char,length + 1);
    memcpy(heapChars,chars,length);
    heapChars[length] = '\0';
    return allocateString(heapChars,length);
}


void printObject(Value value){
    switch(OBJ_TYPE(value)){
        case OBJ_STR:
            printf("%s",AS_CSTRING(value));
            break;
        default:
            return;
    }
}