#include "memory.h"
#include "vm.h"
#include "compiler.h"
#include <stdlib.h>

#ifdef GC_LOG
#include <stdio.h>
#include "debug.h"
#endif


void* reallocate(void *pointer,size_t oldSize,size_t newSize){
    
    vm.bytesAllocated += newSize - oldSize;
    
    if(newSize == 0){
        free(pointer);
        return NULL;
    }

    if(newSize > oldSize){
        #ifdef GC_STRESS
        collectGarbage();
        #endif
    }

    if(vm.bytesAllocated > vm.nextGC){
        collectGarbage();
    }

    void *result = realloc(pointer,newSize);
    if(result == NULL)exit(1);
    return result;
}

void markObject(Obj*object){
    if(object == NULL || object->isMarked)return;
    object->isMarked = true;
    #ifdef GC_LOG
    printf("%p mark ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
    #endif

    if(vm.grayCount + 1 > vm.grayCapacity){
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Obj**)realloc(vm.grayStack,sizeof(Obj*) * vm.grayCapacity);
        if(vm.grayStack == NULL)exit(1);
    }
    vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value){
    if(IS_OBJ(value))markObject(AS_OBJ(value));
}

void markTable(Table *table){
    for(int i = 0;i < table->capacity;i++){
        Entry *entry = &table->entries[i];
        markObject((Obj*)entry->key);
        markValue(entry->value);
    }
}


void markRoots(){
    // marking all objects on the stack
    for(Value *slot = vm.stack;slot < vm.stackTop;slot++){
        markValue(*slot);
    }

    // marking the objects in the vm global table
    markTable(&vm.globals);

    // marking the functions in the vm callframes
    for(int i = 0;i < vm.frameCount;i++){
        markObject((Obj*)vm.frames[i].function);
    }

    //marking the objects held by the compiler
    markCompilerRoots();
}

void markArray(ValueArray *array){
    for(int i = 0;i < array->size;i++){
        markValue(array->values[i]);
    }
}

void blackenObject(Obj*obj){
    switch(obj->type){
        case OBJ_NATIVE:
        case OBJ_STR:
             break;
        case OBJ_FUNCTION:{
            ObjFunction*function = (ObjFunction*)obj;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_CLASS :{
            ObjClass*klass = (ObjClass*)obj;
            markObject((Obj*)klass->name);
            break;
        }
        case OBJ_INSTANCE :{
            ObjInstance *instance = (ObjInstance*)obj;
            markObject((Obj*)instance->klass);
            markTable(&instance->fields);
            break;
        }
    }
    #ifdef GC_LOG
    printf("%p blacken ", (void*)obj);
    printValue(OBJ_VAL(obj));
    printf("\n");
    #endif
}

void traceReferences(){
    while(vm.grayCount > 0){
        Obj*obj = vm.grayStack[--vm.grayCount];
        blackenObject(obj);
    }
}

void sweep(){
    Obj*object = vm.objects;
    Obj*prev = NULL;
    while(object != NULL){
        if(object->isMarked){
            object->isMarked = false;
            prev = object;
            object = object->next;
        }
        else{
            Obj*unreachable = object;
            object = object->next;
            if(prev == NULL){
                vm.objects = object;
            }
            else{
                prev->next = object;
            }
            freeObject(unreachable);
        }
    }
}

void tableRemoveWhite(Table*table){
    for(int i = 0;i < table->capacity;i++){
        Entry *entry = &table->entries[i];
        if(entry->key != NULL && !entry->key->obj.isMarked){
            tableDelete(table,entry->key);
        }
    }
}

void collectGarbage(){
    #ifdef GC_LOG
    printf("--gc begin\n");
    size_t before = vm.bytesAllocated;
    #endif

    markRoots();
    traceReferences();
    tableRemoveWhite(&vm.strings);
    sweep();

    vm.nextGC = vm.bytesAllocated * GC_GROW_RATE;

    #ifdef GC_LOG
    printf("--gc end \n");
    printf("Collected %zu bytes (from %zu to %zu) next at %zu\n",before - vm.bytesAllocated,before,vm.bytesAllocated,vm.nextGC);
    #endif
}