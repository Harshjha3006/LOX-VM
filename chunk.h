#ifndef chunk_h
#define chunk_h

#include "common.h"
#include "value.h"

// Opcodes 
typedef enum{

    // OpCode 
    OP_RETURN,

    // OpCode constantIndex
    OP_CONSTANT,

    // OpCode
    OP_NEGATE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

}OpCode;


// struct for a dynamic array of bytecode
typedef struct{

    // total capacity of the dynamic array
    int capacity; 
    // current size of dynamic array
    int size;
    // pointer to the bytecode sequence
    uint8_t*code;
    // dynamic array contaning the constans of the program
    ValueArray constants;
    // dynamic array containing the line numbers where a specific bytecode came from
    int *lines;

}Chunk;

// initialise a chunk
void initChunk(Chunk *chunk);

// append a byte to a chunk

void writeChunk(Chunk *chunk,uint8_t byte,int line);

// freeing a chunk

void freeChunk(Chunk *chunk);

// adds a constant in the constants array of the chunk and returns its index

int addConstant(Chunk *chunk,Value value);

#endif