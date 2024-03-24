#include "vm.h"
#include "debug.h"
#include <stdio.h>

VM vm;

void initVM(){

}
void freeVM(){

}

InterpretResult run(){
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            disAssembleInstruction(vm.chunk,(int)(vm.ip - vm.chunk->code));
        #endif
        uint8_t code = READ_BYTE();
        switch(code){
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                return INTERPRET_OK;
            default:
                return INTERPRET_RUNTIME_ERROR;
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
}


InterpretResult interpret(Chunk *chunk){
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}
