#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "value.h"
#include "object.h"
#include<string.h>
#ifdef DEBUG_PRINT_EXECUTION
#include "debug.h"
#endif

Parser parser;
Chunk *compilingChunk;
Compiler*current = NULL;
void statement();
void declaration();

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

int emitJump(uint8_t instruction){
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->size - 2;
}

void emitLoop(int loopStart){
    emitByte(OP_LOOP);

    int jump = currentChunk()->size - loopStart + 2;
    if(jump > UINT16_MAX){
        errorAtPrevious("Loop body too large");
    }
    
    emitByte((uint8_t)(jump << 8));
    emitByte((uint8_t)(jump));

}

void patchJump(int offset){
    int jump = currentChunk()->size - offset - 2;

    if(jump > UINT16_MAX){
        errorAtPrevious("Too Much large jump");
    }
    currentChunk()->code[offset] = (uint8_t)(jump >> 8); // MSB 
    currentChunk()->code[offset + 1] = (uint8_t)(jump); // LSB
}

void endCompiler(){
    emitReturn();
    #ifdef DEBUG_PRINT_EXECUTION
        disAssembleChunk(currentChunk(),"code");
    #endif
}

static bool check(TokenType type){
    return parser.current.type == type;
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

void synchronise(){
    parser.panicMode = false;
    while(parser.current.type != TOKEN_EOF){
        if(parser.previous.type == TOKEN_SEMICOLON)return;
        switch(parser.current.type){
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default: 
        }
        advance();
    }
}

static bool match(TokenType type){
    if(!check(type))return false;
    advance();
    return true;
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

    bool canAssign = precdence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while(precdence <= getRule(parser.current.type)->precedence){
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infixRule;
        infixRule(canAssign);
    }
    if(canAssign && match(TOKEN_EQUAL)){
        errorAtCurrent("invalid assignment");
    }
}

void expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

void number(bool canAssign){
    double value = strtod(parser.previous.start,NULL);
    emitConstant(NUM_VAL(value));
}

void literal(bool canAssign){
    switch(parser.previous.type){
        case TOKEN_TRUE: emitByte(OP_TRUE);break;
        case TOKEN_FALSE: emitByte(OP_FALSE);break;
        case TOKEN_NIL: emitByte(OP_NIL);break;
        default: return;
    }
}

static void string(bool canAssign){
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,parser.previous.length - 2)));
}

void grouping(bool canAssign){
    expression();
    consume(TOKEN_RIGHT_PAREN,"Expect ) at the end of expression");
}

void unary(bool canAssign){
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

void binary(bool canAssign){
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

void _and(bool canAssign){
    int offset = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    patchJump(offset);
}

void _or(bool canAssign){
    int offset = emitJump(OP_JUMP_IF_FALSE);
    int endOffset = emitJump(OP_JUMP);
    patchJump(offset);
    emitByte(OP_POP);
    parsePrecedence(PREC_OR);
    patchJump(endOffset);

}

uint8_t identifierConstant(Token *name){
    return makeConstant(OBJ_VAL(copyString(name->start,name->length)));
}
bool identifiersEqual(Token*a,Token*b){
    if(a->length != b->length)return false;

    return memcmp(a->start,b->start,a->length) == 0;
}

int resolveLocal(Compiler*compiler,Token *name){

    for(int i = compiler->localCount - 1;i >= 0;i--){
        Local*local = &compiler->locals[i];
        if(identifiersEqual(name,&local->name)){
            if(local->depth == -1){
                errorAtPrevious("Can't initialise a uninitialised variable to itself");
            }
            return i;
        }
    }
    return -1;
}

void namedVariable(Token name,bool canAssign){
    uint8_t global = identifierConstant(&parser.previous);

    uint8_t getOp,setOp;
    int arg = resolveLocal(current,&name);
    if(arg != -1){
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else{
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if(canAssign && match(TOKEN_EQUAL)){
        expression();
        emitBytes(setOp,(uint8_t)arg);
    }
    else{
        emitBytes(getOp,(uint8_t)arg);
    }
}

void variable(bool canAssign){
    namedVariable(parser.previous,canAssign);
}

void addLocal(Token name){

    if(current->localCount == UINT8_MAX ){
        errorAtPrevious("Too Many local variables declared in a block");
        return;
    }

    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}



void declareVariable(){
    if(current->scopeDepth == 0)return;

    Token*name = &parser.previous;

    for(int i = current->localCount - 1;i >= 0;i--){
        Local *local = &current->locals[i];
        if(local->depth != -1 && local->depth < current->scopeDepth){
            break;
        }
        if(identifiersEqual(&local->name,name)){
            errorAtPrevious("Already a variable with this name in this scope");
        }
    }

    addLocal(*name);
}

uint8_t parseVariableName(const char*message){
    consume(TOKEN_IDENTIFIER,message);

    declareVariable();
    if(current->scopeDepth > 0){
        return 0;
    }
    return identifierConstant(&parser.previous);
}

void markInitialised(){
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

void defineVariable(uint8_t global){
    if(current->scopeDepth > 0){
        markInitialised();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL,global);
}

void varDeclaration(){
    uint8_t global = parseVariableName("Expected Variable Name");

    if(match(TOKEN_EQUAL)){
        expression();
    }
    else{
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON,"Expected ; at the end");
    defineVariable(global);
}

void printStatement(){
    expression();
    consume(TOKEN_SEMICOLON,"Expected ; at the end");
    emitByte(OP_PRINT);
}

void expressionStatement(){
    expression();
    consume(TOKEN_SEMICOLON,"Expected ; at the end");
    emitByte(OP_POP);
}

void block(){
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE,"Expected } at the end");
}
void beginScope(){
    current->scopeDepth++;
}
void endScope(){
    current->scopeDepth--;
    uint8_t delCount = 0;
    while(current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth){
        current->localCount--;
        delCount++;
    }
    emitBytes(OP_POPN,delCount);
}


void ifStatement(){
    consume(TOKEN_LEFT_PAREN,"Expected ( after if");
    expression();
    consume(TOKEN_RIGHT_PAREN,"Expected ) after condition");

    int thenOffset = emitJump(OP_JUMP_IF_FALSE);
    
    emitByte(OP_POP);

    statement();

    int elseOffset = emitJump(OP_JUMP);

    emitByte(OP_POP);

    patchJump(thenOffset);

    if(match(TOKEN_ELSE))statement();

    patchJump(elseOffset);

}

void whileStatement(){
    int loopStart = currentChunk()->size;
    consume(TOKEN_LEFT_PAREN,"Expect ( after while");
    expression();
    consume(TOKEN_RIGHT_PAREN,"Expect ) after condition");

    int offset = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);
    patchJump(offset);
    emitByte(OP_POP);

}

void forStatement(){
    beginScope();
    consume(TOKEN_LEFT_PAREN,"Expected ( after for");

    if(match(TOKEN_SEMICOLON)){
        // no initialiser, do nothing
    }
    else if(match(TOKEN_VAR)){
        varDeclaration();
    }
    else{
        expressionStatement();
    }

    int exitJump = -1;
    int loopStart = currentChunk()->size;
    if(!match(TOKEN_SEMICOLON)){
        // there is a condition
        expression();
        consume(TOKEN_SEMICOLON,"Expected ; after condition");
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    if(!match(TOKEN_RIGHT_PAREN)){
        // there is an increment clause
        int incrementJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->size;

        expression();
        consume(TOKEN_RIGHT_PAREN,"Expected ) after increment clause");
        emitByte(OP_POP);

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(incrementJump);
    }
    statement();
    emitLoop(loopStart);
    
    if(exitJump != -1){
        patchJump(exitJump);
        emitByte(OP_POP);
    }

    endScope();
}

void statement(){
    if(match(TOKEN_PRINT)){
        printStatement();
    }
    else if(match(TOKEN_LEFT_BRACE)){
        beginScope();
        block();
        endScope();
    }
    else if(match(TOKEN_IF)){
        ifStatement();
    }
    else if(match(TOKEN_WHILE)){
        whileStatement();
    }
    else if(match(TOKEN_FOR)){
        forStatement();
    }
    else{
        expressionStatement();
    }
}


void declaration(){
    if(match(TOKEN_VAR)){
        varDeclaration();
    }
    else{
        statement();
    }
    if(parser.panicMode == true)synchronise();
}

void initCompiler(Compiler *compiler){
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

bool compile(const char*source,Chunk *chunk){
    initScanner(source);
    parser.hadError = false;
    parser.panicMode = false;
    compilingChunk = chunk;
    Compiler compiler;
    initCompiler(&compiler);
    advance();

    while(!match(TOKEN_EOF)){
        declaration();
    }
    // expression();
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
  [TOKEN_IDENTIFIER]    = {variable,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     _and,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     _or,   PREC_OR},
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