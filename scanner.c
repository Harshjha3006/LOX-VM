#include "scanner.h"
#include "common.h"
#include <string.h>
#include <stdio.h>

Scanner scanner;


void initScanner(const char*source){
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

// checks if we are at end of source code
bool isAtEnd(){
    return *scanner.current == '\0';
}


// return a Token for the given TokenType
Token makeToken(TokenType type){
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

// returns the current char and increments the current pointer
static char advance(){
    scanner.current++;
    return scanner.current[-1];
}

// returns a TOKEN_ERROR token for the given error message
Token errorToken(const char*message){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

// consumes the current char if it is equal to the expected character
bool match(char expected){
    if(isAtEnd())return false;
    if(*scanner.current == expected){
        scanner.current++;
        return true;
    }
    return false;
}

// returns the current character
char peek(){
    return *scanner.current;
}

// returns the next character
char peekNext(){
    if(isAtEnd())return '\0';
    return scanner.current[1];
}

// consumes whitespaces
void skipWhiteSpace(){
   for(;;){
    char c = peek();
    switch(c){
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            scanner.line++;
            advance();
            break;
        default:
            return;
    }
   }
}

// handles strings enclosed within ""
static Token string(){
    while(peek() != '"' && !isAtEnd()){
        if(peek() == '\n')scanner.line++;
        advance();
    }

    if(isAtEnd())return errorToken("Unterminated string");
    advance();
    return makeToken(TOKEN_STRING);
}

// checks if the character is a digit
bool isDigit(char c){
    return c <= '9' && c >= '0';
}

// checks if the character is an alphabet
bool isAlpha(char c){
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')return true;
    return false;
}

// handles integer and floating point numbers
static Token number(){
    while(isDigit(peek())){
        advance();
    }

    // handles case for floating point numbers
    if(peek() == '.' && isDigit(peekNext())){
        advance();
        while(isDigit(peek()))advance();
    }
    return makeToken(TOKEN_NUMBER);
}

// checks if the "rest" string is present after the start index
TokenType checkKeyword(int start,int length,const char*rest,TokenType type){
    if((scanner.current - scanner.start) == start + length && memcmp(scanner.start + start,rest,length) == 0){
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// returns a keyword or a regular identifier

TokenType identifierType(){

    switch(scanner.start[0]){
        case 'a': return checkKeyword(1,2,"nd",TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
      break;
    }

    return TOKEN_IDENTIFIER; 
}


Token identifier(){
    while(isAlpha(peek()) || isDigit(peek()))advance();
    return makeToken(identifierType());
}

// scans one token and returns it
Token scanToken(){
    skipWhiteSpace();
    scanner.start = scanner.current;
    if(isAtEnd())return makeToken(TOKEN_EOF);

    char c = advance();
    if(isAlpha(c)){
        return identifier();
    }
    if(isDigit(c))return number();

    switch(c){
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case '+': return makeToken(TOKEN_PLUS);
        case '-': return makeToken(TOKEN_MINUS);
        case '*': return makeToken(TOKEN_STAR);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '!':
            return makeToken(match('=')?TOKEN_BANG_EQUAL:TOKEN_BANG);
        case '<':
            return makeToken(match('=')?TOKEN_LESS_EQUAL:TOKEN_LESS);
        case '>':
            return makeToken(match('=')?TOKEN_GREATER_EQUAL:TOKEN_GREATER);
        case '=':
            return makeToken(match('=')?TOKEN_EQUAL_EQUAL:TOKEN_EQUAL);
        case '/':
            if(peekNext() == '/'){
                while(peek() != '\n' && !isAtEnd()){
                    advance();
                }
            }
            else{
                return makeToken(TOKEN_SLASH);
            }
            break;
        case '"':return string();
    }

    return errorToken("Unexpected Character");

}