#ifndef vm_h
#define vm_h
// #define DEBUG_TRACE_EXECUTION
#include "chunk.h"
#include "value.h"
#include "table.h"
#define FRAME_MAX 64
#define STACK_SIZE (FRAME_MAX * UINT8_MAX)


typedef struct{
    ObjFunction*function;
    uint8_t*ip;
    Value *slots;
}CallFrame;

// struct for the virtual machine 
typedef struct {
    CallFrame frames[FRAME_MAX];

    int frameCount;
    // stack
    Value stack[STACK_SIZE];
    // pointer to top of the stack
    Value *stackTop;
    // pointer to linked list of dynamically allocated objects
    Obj*objects;
    // Hashset of interned strings
    Table strings;
    // Hashmap of global variables
    Table globals;
}VM;

extern VM vm;

// initialising the stack
void initVM();
// freeing the stack
void freeVM();
// pushing a value on top of the stack
void push(Value value);
// popping a value from the top of the stack
Value pop();


// enum for different types of results returned by the interpreter
typedef enum{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
}InterpretResult;


// interprets the given source code
InterpretResult interpret(const char*source);


#endif