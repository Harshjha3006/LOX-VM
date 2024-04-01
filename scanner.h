#ifndef scanner_h
#define scanner_h


// struct for the lexical scanner
typedef struct{
    // pointer to start of current token
    const char*start;
    // pointer to current char where the scanner is at
    const char*current;
    // current line number
    int line;
}Scanner;


// enum for various token types
typedef enum{
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
  TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,
  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
  // Keywords.
  TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
  TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

  TOKEN_ERROR, TOKEN_EOF

}TokenType;


// struct for a token
typedef struct{
    // type of a token
    TokenType type;
    // pointer to start of its lexeme
    const char*start;
    // length of the lexeme
    int length;
    // its line number
    int line;
}Token;


// initialises the scanner
void initScanner(const char*source);

// scans one token and returns it
Token scanToken();



#endif