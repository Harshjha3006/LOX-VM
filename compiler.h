#ifndef compiler_h
#define compiler_h
#include "chunk.h"
#include "scanner.h"
#define DEBUG_PRINT_EXECUTION

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

typedef struct{
    Local locals[UINT8_MAX + 1];
    int localCount;
    int scopeDepth;
}Compiler;

typedef void(*ParseFn)(bool canAssign);

typedef struct{
    ParseFn prefixRule;
    ParseFn infixRule;
    Precedence precedence;
}ParseRule;

ParseRule* getRule(TokenType type);

bool compile(const char*source,Chunk *chunk);


#endif