#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "value.h"
#include "object.h"
#ifdef DEBUG_PRINT_EXECUTION
#include "debug.h"
#endif

Parser parser;
Chunk *compilingChunk;

void error(Token *token,const char*message){
    if(parser.panicMode == true)return;
    parser.panicMode = true;
    fprintf(stderr,"Compiler Error : line [%d]: ",token->line);
    fprintf(stderr,"at '%.*s'",token->length,token->start);
    fprintf(stderr,": %s\n",message);
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
    emitConstant(NUM_VAL(value));
}

void literal(){
    switch(parser.previous.type){
        case TOKEN_TRUE: emitByte(OP_TRUE);break;
        case TOKEN_FALSE: emitByte(OP_FALSE);break;
        case TOKEN_NIL: emitByte(OP_NIL);break;
        default: return;
    }
}

static void string(){
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,parser.previous.length - 2)));
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
        case TOKEN_BANG:
            emitByte(OP_NOT);
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
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL,OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESSER);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER,OP_NOT);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESSER,OP_NOT);
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
  [TOKEN_BANG]          = {unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISION},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISION},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISION},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISION},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

ParseRule* getRule(TokenType type){
    return &rules[type];
}