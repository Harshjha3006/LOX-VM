#ifndef vm_h
#define vm_h
#define DEBUG_TRACE_EXECUTION
#include "chunk.h"
#include "value.h"
#define STACK_SIZE 256

typedef struct {
    Chunk*chunk;
    uint8_t* ip;
    Value stack[STACK_SIZE];
    Value *stackTop;
}VM;

void initVM();
void freeVM();
void push(Value value);
Value pop();

typedef enum{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
}InterpretResult;


InterpretResult interpret(Chunk*chunk);


#endif