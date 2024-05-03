#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REPL_LINE_SIZE 1024


// starts a repl
void repl(){
    char line[REPL_LINE_SIZE]; // to store the single line input
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

// reads the source file into a buffer and returns the buffer
char *readFile(const char*path){

    /*
        file is opened ,fileSize is calculated by moving the file pointer to the end of the file 
        and then a sufficiently large buffer is allocated 
    */

    FILE*file = fopen(path,"rb");
    if(file == NULL){
        fprintf(stderr,"Could not open file %s\n",path);
        exit(30);
    }
    fseek(file,0,SEEK_END);
    size_t fileSize = ftell(file);
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

// runs the source file specified by the path
void runFile(const char *path){
    // reading the file
    char*source = readFile(path);
    // running the code and getting the result
    InterpretResult result = interpret(source);
    // freeing the pointer
    free(source);

    // compiler error
    if(result == INTERPRET_COMPILE_ERROR)exit(72);
    
    // runtime error 
    if(result == INTERPRET_RUNTIME_ERROR)exit(73);
}


int main(int argc, char **argv){


    // initialise the VM
    initVM();

    // repl if no arguement
    if(argc == 1){
        repl();
    }
    // runs source file if 1 arguement is specfied
    else if(argc == 2){
        runFile(argv[1]);
    }
    // otherwise error and prints usage
    else{
        fprintf(stderr,"Usage : clox [path]");
    }

    // deallocates resources owned by the VM
    freeVM();

    return 0;
}