#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include <stdio.h>

VM vm;

void initVM(){
    vm.stackTop = vm.stack;
}

void push(Value value){
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}


void freeVM(){

}

InterpretResult run(){
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define BINARY_OP(op)\
        do{\
            double b = pop();\
            double a = pop();\
            push(a op b);\
        }while(false)

    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            printf("              ");
            for(Value *slot = vm.stack;slot < vm.stackTop;slot++){
                printf("[");
                printValue(*slot);
                printf("]");
            }
            printf("\n");
            disAssembleInstruction(vm.chunk,(int)(vm.ip - vm.chunk->code));
        #endif
        uint8_t code = READ_BYTE();
        switch(code){
            case OP_RETURN:
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            case OP_NEGATE:
                push(-pop());
                break;
            case OP_ADD:
                BINARY_OP(+);
                break;
            default:
                return INTERPRET_RUNTIME_ERROR;
        }
    }
    return INTERPRET_OK;

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}


InterpretResult interpret(const char*source){
    compile(source);

    return INTERPRET_OK;
}
