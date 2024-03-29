#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void repl(){
    char line[1024]; // to store the single line input
    for(;;){
        printf("> ");
        if(!fgets(line,sizeof(line),stdin)){
            printf("\n");
            break;
        }
        interpret(line);
    }
}

char *readFile(const char*path){
    FILE*file = fopen(path,"rb");
    if(file == NULL){
        fprintf(stderr,"Could not open file %s\n",path);
        exit(30);
    }
    fseek(file,0,SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char*buffer = (char *)malloc(fileSize + 1);
    if(buffer == NULL){
        fprintf(stderr,"Could not allocate more memory\n");
        exit(30);
    }
    size_t bytesRead = fread(buffer,sizeof(char),fileSize,file);
    if(bytesRead < fileSize){
        fprintf(stderr,"Could not read file %s",path);
        exit(30);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

void runFile(const char *path){
    char*source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);
    if(result == INTERPRET_COMPILE_ERROR)exit(72);
    if(result == INTERPRET_RUNTIME_ERROR)exit(73);
}

int main(int argc, char **argv){

    initVM();

    if(argc == 1){
        repl();
    }
    else if(argc == 2){
        runFile(argv[1]);
    }
    else{
        fprintf(stdin,"Usage : clox [path]");
    }

    return 0;
}