#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "value.h"
#include "object.h"
#include <inttypes.h>

VM vm;

// points the stack top pointer to the beginning of the stack array
void resetStack(){
    vm.stackTop = vm.stack;
}

void initVM(){
    resetStack();
    vm.objects = NULL;
    initTable(&vm.strings);
    initTable(&vm.globals);
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

void freeObject(Obj*obj){
    switch(obj->type){
        case OBJ_STR:
            ObjString*string = (ObjString*)obj;
            FREE_ARRAY(char,string->chars,string->length + 1);
            FREE(ObjString,string);
            break;
        default:
            return;
    }
}

void freeObjects(Obj*objects){
    Obj*obj = objects;
    while(obj != NULL){
       Obj*next = obj->next;
       freeObject(obj);
       obj = next;
    }
}

void freeVM(){
    freeObjects(vm.objects);
    freeTable(&vm.strings);
    freeTable(&vm.globals);
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
        case VAL_OBJ:
             return AS_OBJ(a) == AS_OBJ(b);
        default:
            return false;
    }
}

ObjString* takeString(char *chars,int length){
    uint32_t hash = hashString(chars,length);
    ObjString *interned = tableFindString(&vm.strings,chars,length,hash);
    if(interned != NULL){
        FREE_ARRAY(char,chars,length + 1);
        return interned;
    }
    return allocateString(chars,length,hash);
}

void concatenate(){
    ObjString * b = AS_STRING(pop());
    ObjString * a = AS_STRING(pop());
    int length = a->length + b->length;
    char*chars = ALLOCATE(char,length + 1);
    memcpy(chars,a->chars,a->length);
    memcpy(chars + a->length,b->chars,b->length);
    chars[length] = '\0';
    ObjString*result = takeString(chars,length);
    push(OBJ_VAL(result));
}

InterpretResult run(){
    // returns the byte at instruction pointer and increments the ip 
    #define READ_BYTE() (*vm.ip++)
    // returns the constant at the current index in the bytecode
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    // returns the string at the current index in the bytecode
    #define READ_STRING() AS_STRING(READ_CONSTANT())
    // combines two 8 bit operands as a single 16 bit number
    #define READ_SHORT() (vm.ip += 2,(uint16_t)(vm.ip[-2] << 8 | vm.ip[-1]))

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
            case OP_POP:
                pop();
                break;
            case OP_RETURN:
                // printValue(pop());
                // printf("\n");
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
                if(IS_STRING(peek(0)) && IS_STRING(peek(1))){
                    concatenate();
                }
                else if(IS_NUM(peek(0)) && IS_NUM(peek(1))){
                    BINARY_OP(NUM_VAL,+);
                }
                else{
                    runtimeError("Operands should be either strings or numbers");
                    return INTERPRET_RUNTIME_ERROR;
                }
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
                BINARY_OP(BOOL_VAL,>);
                break;
            case OP_LESSER:
                BINARY_OP(BOOL_VAL,<);
                break;
            case OP_PRINT:
                printValue(pop());
                printf("\n");
                break;
            case OP_DEFINE_GLOBAL:{
                ObjString*key = READ_STRING();
                tableSet(&vm.globals,key,peek(0));
                pop();
                break;
            }
            case OP_GET_GLOBAL:{
                ObjString*key = READ_STRING();
                Value value;
                if(!tableGet(&vm.globals,key,&value)){
                    runtimeError("Undefined Variable : %.*s",key->length,key->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_SET_GLOBAL:{
                ObjString *key = READ_STRING();
                if(tableSet(&vm.globals,key,peek(0))){
                    tableDelete(&vm.globals,key);
                    runtimeError("Undefined Variable : %.*s",key->length,key->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_LOCAL:{
                uint8_t slot = READ_BYTE();
                push(vm.stack[slot]);
                break;
            }
            case OP_SET_LOCAL:{
                uint8_t slot = READ_BYTE();
                vm.stack[slot] = peek(0);
                break;
            }
            case OP_POPN:
                uint8_t arg = READ_BYTE();
                while(arg--){
                    pop();
                }
                break;
            case OP_JUMP_IF_FALSE:{
                uint16_t offset = READ_SHORT();
                if(isFalsey(peek(0))){
                    vm.ip += offset;
                }
                break;
            }
            case OP_JUMP:{
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }
            case OP_LOOP:{
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }
            default:
                return INTERPRET_RUNTIME_ERROR;
        }
    }
    return INTERPRET_OK;

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
    #undef READ_STRING
    #undef READ_SHORT
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
    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;
    // Running the VM
    InterpretResult result = run();
    freeChunk(&chunk);
    return result;
}
