#ifndef compiler_h
#define compiler_h
#include "chunk.h"
#include "scanner.h"

typedef struct{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
}Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISION,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
}Precedence;

typedef struct{
    Token name;
    int depth;
}Local;

typedef enum{
    FUNC_MAIN,
    FUNC_USER,
    FUNC_METHOD,
    FUNC_INITIALIZER,
}FunctionType;

typedef struct Compiler{
    struct Compiler *enclosing; // compiler which called this compiler
    ObjFunction*function; // current function which it is compiling
    FunctionType type; // type of function 
    Local locals[UINT8_MAX + 1]; // array of local variables
    int localCount;
    int scopeDepth;
}Compiler;

typedef struct ClassCompiler{
    struct ClassCompiler* enclosing;
}ClassCompiler;

typedef void(*ParseFn)(bool canAssign);

typedef struct{
    ParseFn prefixRule;
    ParseFn infixRule;
    Precedence precedence;
}ParseRule;

ParseRule* getRule(TokenType type);

ObjFunction* compile(const char*source);

void markCompilerRoots();

#endif