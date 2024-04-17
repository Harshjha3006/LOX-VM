#include "object.h"
#include "vm.h"
#include <string.h>
#include <stdio.h>
#include "table.h"
#include <inttypes.h>


static Obj* allocateObject(size_t size,ObjType type){
    Obj *obj = (Obj*)reallocate(NULL,0,size);
    obj->type = type;
    obj->next = vm.objects;
    vm.objects = obj;
    return obj;
}

#define ALLOCATE_OBJ(type,objectType) (type*)allocateObject(sizeof(type),objectType)


ObjString *allocateString(char *chars,int length,uint32_t hash){
    ObjString*string = ALLOCATE_OBJ(ObjString,OBJ_STR);
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    tableSet(&vm.strings,string,NIL_VAL);
    return string;
}

// FNV-1a Hashing algorithm

uint32_t hashString(const char*key,int length){
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}
ObjString* tableFindString(Table*table,const char*chars,int length,uint32_t hash){

    if(table->count == 0)return NULL;
    
    uint32_t bucket = hash % table->capacity;
    for(;;){
        Entry*entry = &table->entries[bucket];
        if(entry->key == NULL){
            if(IS_NIL(entry->value))return NULL;
        }
        else if(entry->key->hash == hash && entry->key->length == length 
        && (memcmp(entry->key->chars,chars,length)) == 0){
            return entry->key;
        }
        bucket = (bucket + 1) % table->capacity;
    }
}

ObjString* copyString(const char *chars,int length){
    char *heapChars = ALLOCATE(char,length + 1);
    memcpy(heapChars,chars,length);
    heapChars[length] = '\0';
    uint32_t hash = hashString(chars,length);
    ObjString*interned = tableFindString(&vm.strings,chars,length,hash);
    if(interned != NULL)return interned;
    return allocateString(heapChars,length,hash);
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