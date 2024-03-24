#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, char **argv){

    Chunk chunk;
    initChunk(&chunk);
    
    int index = addConstant(&chunk,1.2);
    writeChunk(&chunk,OP_CONSTANT,123);
    writeChunk(&chunk,index,123);
    index = addConstant(&chunk,3.4);
    writeChunk(&chunk,OP_CONSTANT,123);
    writeChunk(&chunk,index,123);
    writeChunk(&chunk,OP_ADD,123);
    writeChunk(&chunk,OP_RETURN,123);
    initVM();
    interpret(&chunk);
    freeVM();
    freeChunk(&chunk);

    return 0;
}