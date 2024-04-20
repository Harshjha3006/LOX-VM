#include "debug.h"
#include "value.h"
#include <stdio.h>


void disAssembleChunk(Chunk *chunk,const char *name){
    printf("===%s===\n",name);

    for(int offset = 0;offset < chunk->size;){
        offset = disAssembleInstruction(chunk,offset);
    }
}


// for OPCODE
int simpleInstruction(const char *name,int offset){
    printf("%s\n",name);
    return offset + 1;
}

// for OPCODE INDEX
int constantInstruction(const char *name,Chunk *chunk,int offset){

    int constant = chunk->code[offset + 1];
    printf("%s %d ",name,constant);
    Value value = chunk->constants.values[constant];
    printValue(value);
    printf("\n");
    return offset + 2;
}

int byteInstruction(const char *name,Chunk *chunk,int offset){
    int slot = chunk->code[offset + 1];
    printf("%s %d\n",name,slot);
    return offset + 2;
}

int jumpInstruction(const char*name,int sign,Chunk*chunk,int offset){
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= (chunk->code[offset + 2]);
    printf("%s %4d -> %d\n",name,offset,offset + 3 + sign*jump);
    return offset + 3;
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
        case OP_NOT:
            return simpleInstruction("OP_NOT",offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE",offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE",offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL",offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL",offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER",offset);
        case OP_LESSER:
            return simpleInstruction("OP_LESSER",offset);
        case OP_PRINT:
            return simpleInstruction("OP_PRINT",offset);
        case OP_POP:
            return simpleInstruction("OP_POP",offset);
        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL",chunk,offset);
        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL",chunk,offset);
        case OP_SET_GLOBAL:
            return constantInstruction("OP_SET_GLOBAL",chunk,offset);
        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL",chunk,offset);
        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL",chunk,offset);
        case OP_POPN:
            return byteInstruction("OP_POPN",chunk,offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE",1,chunk,offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP",1,chunk,offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP",-1,chunk,offset);
        case OP_CALL:
            return byteInstruction("OP_CALL",chunk,offset);
        default:
            printf("Unknown opcode %d\n",instruction);
            return offset + 1;
    }
}
