#include "compiler.h"
#include "scanner.h"
#include <stdio.h>


void compile(const char*source){
    initScanner(source);

    int line = -1;
    for(;;){
        Token token = scanToken();
        if(token.line != line){
            line = token.line;
            printf("%d ",line);
        }
        else{
            printf("   |");
        }
        printf("%2d '%.*s'\n",token.type,token.length,token.start);
        if(token.type == TOKEN_EOF)break;
    }
}