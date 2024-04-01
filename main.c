#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// starts a repl
void repl(){
    char line[1024]; // to store the single line input
    for(;;){
        printf("> ");
        // scanning input
        if(!fgets(line,sizeof(line),stdin)){
            printf("\n");
            break;
        }
        interpret(line);
    }
}

// reads the source file
char *readFile(const char*path){
    // opens a file
    FILE*file = fopen(path,"rb");
    if(file == NULL){
        fprintf(stderr,"Could not open file %s\n",path);
        exit(30);
    }
    // moving file pointer to end of file
    fseek(file,0,SEEK_END);
    // finding size of file
    size_t fileSize = ftell(file);
    // moving the file pointer back at the start of file
    rewind(file);

    // buffer for storing file content
    char*buffer = (char *)malloc(fileSize + 1);
    if(buffer == NULL){
        fprintf(stderr,"Could not allocate more memory\n");
        exit(30);
    }

    // reading the file
    size_t bytesRead = fread(buffer,sizeof(char),fileSize,file);
    if(bytesRead < fileSize){
        fprintf(stderr,"Could not read file %s",path);
        exit(30);
    }
    // terminating the file content by null character
    buffer[bytesRead] = '\0';

    // closing the file
    fclose(file);
    return buffer;
}

void runFile(const char *path){
    // reading the file
    char*source = readFile(path);
    // running the code and getting the result
    InterpretResult result = interpret(source);
    // freeing the pointer
    free(source);
    if(result == INTERPRET_COMPILE_ERROR)exit(72);
    if(result == INTERPRET_RUNTIME_ERROR)exit(73);
}

int main(int argc, char **argv){

    initVM();

    // repl if no arguement
    if(argc == 1){
        repl();
    }
    // runs source file if 1 arguement is specfied
    else if(argc == 2){
        runFile(argv[1]);
    }
    else{
        fprintf(stdin,"Usage : clox [path]");
    }

    return 0;
}