#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include <stdio.h>

VM vm;


// points the stack top pointer to the beginning of the stack array
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
    // returns the byte at instruction pointer and increments the ip 
    #define READ_BYTE() (*vm.ip++)
    // returns the constant at the current index in the bytecode
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    // defines binary operations which involves the top 2 values of the stack
    #define BINARY_OP(op)\
        do{\
            double b = pop();\
            double a = pop();\
            push(a op b);\
        }while(false)

    // printing the contents of the stack and the current bytecode
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
            case OP_SUB:
                BINARY_OP(-);
                break;
            case OP_MUL:
                BINARY_OP(*);
                break;
            case OP_DIV:
                BINARY_OP(/);
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

    // initialising chunk struct
    Chunk chunk;
    initChunk(&chunk);

    // checking for compiler error 
    if(!compile(source,&chunk)){
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    // initialising VM
    initVM();
    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    // Running the VM
    InterpretResult result = run();
    freeChunk(&chunk);
    return result;
}
