#include "object.h"
#include "vm.h"
#include <string.h>
#include <stdio.h>
#include "table.h"
#include <inttypes.h>


static Obj* allocateObject(size_t size,ObjType type){
    Obj *obj = (Obj*)reallocate(NULL,0,size);
    obj->type = type;
    obj->isMarked = false;
    obj->next = vm.objects;
    vm.objects = obj;
    #ifdef GC_LOG
    printf("%p allocated %zu bytes of type %d\n",(void*)obj,size,type);
    #endif
    return obj;
}

#define ALLOCATE_OBJ(type,objectType) (type*)allocateObject(sizeof(type),objectType)


ObjString *allocateString(char *chars,int length,uint32_t hash){
    ObjString*string = ALLOCATE_OBJ(ObjString,OBJ_STR);
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    push(OBJ_VAL(string));
    tableSet(&vm.strings,string,NIL_VAL);
    pop();
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
    
    uint32_t bucket = hash & (table->capacity - 1);
    for(;;){
        Entry*entry = &table->entries[bucket];
        if(entry->key == NULL){
            if(IS_NIL(entry->value))return NULL;
        }
        else if(entry->key->hash == hash && entry->key->length == length 
        && (memcmp(entry->key->chars,chars,length)) == 0){
            return entry->key;
        }
        bucket = (bucket + 1) & (table->capacity - 1);
    }
}

ObjString* copyString(const char *chars,int length){
   
    uint32_t hash = hashString(chars,length);
    ObjString*interned = tableFindString(&vm.strings,chars,length,hash);
    if(interned != NULL)return interned;
    
    char *heapChars = ALLOCATE(char,length + 1);
    memcpy(heapChars,chars,length);
    heapChars[length] = '\0';
    return allocateString(heapChars,length,hash);
}

ObjFunction* newFunction(){
    ObjFunction*function = ALLOCATE_OBJ(ObjFunction,OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjNative*newNative(NativeFn fn){
    ObjNative*native = ALLOCATE_OBJ(ObjNative,OBJ_NATIVE);
    native->fn = fn;
    return native;
}

ObjClass *newClass(ObjString*name){
    ObjClass*klass = ALLOCATE_OBJ(ObjClass,OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}

ObjInstance *newInstance(ObjClass *klass){
    ObjInstance *instance = ALLOCATE_OBJ(ObjInstance,OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjBoundMethod *newBoundMethod(ObjFunction*function,Value receiver){
    ObjBoundMethod*method = ALLOCATE_OBJ(ObjBoundMethod,OBJ_BOUND_METHOD);
    method->method = function;
    method->receiver = receiver;
    return method;
}

void printFunction(ObjFunction*function){
    if(function->name == NULL){
        printf("main");
        return;
    }
    printf("<fn %s>",function->name->chars);
}


void printObject(Value value){
    switch(OBJ_TYPE(value)){
        case OBJ_STR:
            printf("%s",AS_CSTRING(value));
            break;
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_CLASS:
            printf("%s",AS_CLASS(value)->name->chars);
            break;
        case OBJ_INSTANCE:
            printf("%s instance",AS_INSTANCE(value)->klass->name->chars);
            break;
        case OBJ_BOUND_METHOD:
            printFunction(AS_BOUND_METHOD(value)->method);
            break;
        default:
            return;
    }
}