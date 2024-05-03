#ifndef vm_h
#define vm_h
#include "chunk.h"
#include "value.h"
#include "table.h"

// specifies max number of callFrames 
#define FRAME_MAX 64
// specifies max size of the VM stack
#define STACK_SIZE (FRAME_MAX * UINT8_MAX)


/*
    CallFrame struct :- it holds runtime information about a function being called 
*/
typedef struct{
    // pointer to function object
    ObjFunction*function;
    // instruction pointer to the function's bytecode chunk
    uint8_t*ip;
    // pointer to stack slots local to the function
    Value *slots;
}CallFrame;

// struct for the virtual machine 
typedef struct {
    // array of vm's callframes
    CallFrame frames[FRAME_MAX];
    // current number of frames
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
    // array of gray objects for the garbage collector
    Obj**grayStack; 
    // current number of gray Objects
    int grayCount;
    // capacity of grayStack
    int grayCapacity;
    // amount of currently allocated memory
    size_t bytesAllocated;
    // memory threshold at which the garbage collector will run
    size_t nextGC;
}VM;

// exporting the VM to other files
extern VM vm;

// initialising the VM
void initVM();
// freeing the VM
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

// frees an object
void freeObject(Obj*obj);

#endif