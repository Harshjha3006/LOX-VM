#ifndef vm_h
#define vm_h
#define DEBUG_TRACE_EXECUTION
#include "chunk.h"

typedef struct {
    Chunk*chunk;
    uint8_t* ip;
}VM;

void initVM();
void freeVM();

typedef enum{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
}InterpretResult;


InterpretResult interpret(Chunk*chunk);


#endif