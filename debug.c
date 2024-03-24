#include "debug.h"
#include "value.h"
#include <stdio.h>


void disAssembleChunk(Chunk *chunk,const char *name){
    printf("===%s===\n",name);

    for(int offset = 0;offset < chunk->size;){
        offset = disAssembleInstruction(chunk,offset);
    }
}

int simpleInstruction(const char *name,int offset){
    printf("%s\n",name);
    return offset + 1;
}

int constantInstruction(const char *name,Chunk *chunk,int offset){

    int constant = chunk->code[offset + 1];
    printf("%s %d ",name,constant);
    Value value = chunk->constants.values[constant];
    printValue(value);
    printf("\n");
    return offset + 2;
}

int disAssembleInstruction(Chunk * chunk,int offset){
    printf("%04d ",offset);

    if(offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]){
        printf("   | ");
    }
    else{
        printf("%d ",chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch(instruction){
        case OP_RETURN:
            return simpleInstruction("OP_RETURN",offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT",chunk,offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE",offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD",offset);
        case OP_SUB:
            return simpleInstruction("OP_SUB",offset);
        case OP_MUL:
            return simpleInstruction("OP_MUL",offset);
        case OP_DIV:
            return simpleInstruction("OP_DIV",offset);
        default:
            printf("Unknown opcode %d\n",instruction);
            return offset + 1;
    }
}
