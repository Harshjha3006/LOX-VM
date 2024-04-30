#include "chunk.h"
#include "vm.h"
#include "memory.h"
#include <stdlib.h>


void initChunk(Chunk *chunk){
    chunk->capacity = 0;
    chunk->size = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}


void writeChunk(Chunk *chunk, uint8_t byte,int line){

    // checking if capacity is full
    if(chunk->size == chunk->capacity){
        // dynamically growing the array 

        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t,chunk->code,oldCapacity,chunk->capacity);
        chunk->lines = GROW_ARRAY(int,chunk->lines,oldCapacity,chunk->capacity);
    }

    chunk->code[chunk->size] = byte;
    chunk->lines[chunk->size] = line;
    chunk->size++;

}


void freeChunk(Chunk *chunk){
    FREE_ARRAY(uint8_t,chunk->code,chunk->capacity);
    freeValueArray(&chunk->constants);
    FREE_ARRAY(int,chunk->lines,chunk->capacity);
    initChunk(chunk);
}

// adding a constant to the values array and returning the index of added constant
int addConstant(Chunk*chunk,Value value){
    push(value);
    writeValueArray(&chunk->constants,value);
    pop();
    return chunk->constants.size - 1;
}