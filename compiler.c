#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef DEBUG_PRINT_EXECUTION
#include "debug.h"
#endif

Parser parser;
Chunk *compilingChunk;

void error(Token *token,const char*message){
    if(parser.panicMode == true)return;
    parser.panicMode = true;
    fprintf(stderr,"line [%d]: ",token->line);
    fprintf(stderr,"%.*s",token->length,message);
    parser.hadError = true;
}

void errorAtCurrent(const char*message){
    error(&parser.current,message);
}

void errorAtPrevious(const char*message){
    error(&parser.previous,message);
}

Chunk* currentChunk(){
    return compilingChunk;
}

uint8_t makeConstant(Value value){
    int constant = addConstant(currentChunk(),value);
    if(constant > UINT8_MAX){
        errorAtPrevious("Too many constants for 1 chunk");
    }
    return (uint8_t)constant;
}

void emitByte(uint8_t byte){
    writeChunk(currentChunk(),byte,parser.previous.line);
}

void emitBytes(uint8_t byte1,uint8_t byte2){
    emitByte(byte1);
    emitByte(byte2);
}

void emitReturn(){
    emitByte(OP_RETURN);
}

void emitConstant(Value value){
    emitBytes(OP_CONSTANT,makeConstant(value));
}

void endCompiler(){
    emitReturn();
    #ifdef DEBUG_PRINT_EXECUTION
        disAssembleChunk(currentChunk(),"code");
    #endif
}

void advance(){
    parser.previous = parser.current;
    
    for(;;){
        parser.current = scanToken();
        if(parser.current.type != TOKEN_ERROR){
            break;
        }
        errorAtCurrent(parser.current.start);
    }
}

void consume(TokenType type,const char*message){
    if(parser.current.type == type){
        advance();
        return;
    }
    errorAtCurrent(message);
}

// here precedence represents the lower bound of precedence for right expressions
void parsePrecedence(Precedence precdence){
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefixRule;
    if(prefixRule == NULL){
        errorAtPrevious("Expect Expression");
        return;
    }
    prefixRule();

    while(precdence <= getRule(parser.current.type)->precedence){
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infixRule;
        infixRule();
    }
}

void expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

void number(){
    double value = strtod(parser.previous.start,NULL);
    emitConstant(value);
}

void grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN,"Expect ) at the end of expression");
}

void unary(){
    TokenType operatorType = parser.previous.type;
    expression();
    switch(operatorType){
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            break;
    }
}

void binary(){
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch(operatorType){
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUB);
            break;
        case TOKEN_STAR:
            emitByte(OP_MUL);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIV);
            break;
        default:
            return;
    }

}

bool compile(const char*source,Chunk *chunk){
    initScanner(source);
    parser.hadError = false;
    parser.panicMode = false;
    compilingChunk = chunk;
    advance();
    expression();
    consume(TOKEN_EOF,"Expect end of expression");
    endCompiler();
    return !parser.hadError;
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

ParseRule* getRule(TokenType type){
    return &rules[type];
}