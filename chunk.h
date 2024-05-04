#ifndef chunk_h
#define chunk_h

#include "common.h"
#include "value.h"

// Opcodes 
typedef enum{

    // OpCode 
    OP_RETURN,

    // OpCode operand1 operand2
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,

    // OpCode constantIndex
    OP_CONSTANT,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_POPN,
    OP_CALL,
    OP_CLASS,
    OP_SET_PROPERTY,
    OP_GET_PROPERTY,
    OP_METHOD,
    OP_INVOKE,

    // OpCode
    OP_NEGATE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESSER,
    OP_PRINT,
    OP_POP,
    OP_TRUE,
    OP_FALSE,
    OP_NIL,

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