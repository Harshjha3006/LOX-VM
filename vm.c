#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include <stdio.h>
#include <stdarg.h>

VM vm;


// points the stack top pointer to the beginning of the stack array
void resetStack(){
    vm.stackTop = vm.stack;
}

void initVM(){
    resetStack();
}

void push(Value value){
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance){
    return vm.stackTop[-1 - distance];
}


void freeVM(){

}

void runtimeError(const char *format,...){
    va_list args;
    va_start(args,format);
    vfprintf(stderr,format,args);
    va_end(args);

    fputs("\n",stderr);
    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr,"[Line %d] error\n",line);
    resetStack();
}

bool isFalsey(Value value){
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

bool areEqual(Value a,Value b){
    if(a.type != b.type)return false;
    switch(a.type){
        case VAL_BOOL:
            return a.as.boolean == b.as.boolean;
        case VAL_NUM:
            return a.as.number == b.as.number;
        case VAL_NIL:
            return true;
    }
}

InterpretResult run(){
    // returns the byte at instruction pointer and increments the ip 
    #define READ_BYTE() (*vm.ip++)
    // returns the constant at the current index in the bytecode
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    // defines binary operations which involves the top 2 values of the stack
    #define BINARY_OP(valueType,op)\
        do{\
            if(!IS_NUM(peek(0)) || !IS_NUM(peek(1))){\
                runtimeError("Operands should be numbers");\
                return INTERPRET_RUNTIME_ERROR;\
            }\
            double b = AS_NUM(pop());\
            double a = AS_NUM(pop());\
            push(valueType(a op b));\
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
                if(!IS_NUM(peek(0))){
                    runtimeError("Operand should be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUM_VAL(-AS_NUM(pop())));
                break;
            case OP_ADD:
                BINARY_OP(NUM_VAL,+);
                break;
            case OP_SUB:
                BINARY_OP(NUM_VAL,-);
                break;
            case OP_MUL:
                BINARY_OP(NUM_VAL,*);
                break;
            case OP_DIV:
                BINARY_OP(NUM_VAL,/);
                break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_TRUE:
                push(BOOL_VAL(true));
                break;
            case OP_FALSE:
                push(BOOL_VAL(false));
                break;
            case OP_NIL:
                push(NIL_VAL);
                break;
            case OP_EQUAL:
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(areEqual(a,b)));
                break;
            case OP_GREATER:
                BINARY_OP(NUM_VAL,>);
                break;
            case OP_LESSER:
                BINARY_OP(NUM_VAL,<);
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
