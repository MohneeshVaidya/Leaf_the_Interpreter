#include <stdio.h>
#include <stdlib.h>


#include "interpreter.h"


void runRepl() {

}


const char *readFile(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        fprintf(stderr, "not able to open or locate file \"%s\"\n", filepath);
        exit(65);
    }

    fseek(file, 0L, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);

    char *buffer = malloc(filesize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "not able to allocate memory for the file \"%s\"\n", filepath);
        fclose(file);
        exit(70);
    }

    size_t bytesread = fread(buffer, sizeof(char), filesize, file);
    if (bytesread <filesize) {
        fprintf(stderr, "not able to read file \"%s\"\n", filepath);
        fclose(file);
        exit(75);
    }
    buffer[filesize] = '\0';

    fclose(file);
    return buffer;
}


void runFile(const char *filepath) {
    const char *source = readFile(filepath);

    InterpretResult result = interpret(source);

    free((void*)source);
    if (result == INTERPRET_COMPILE_ERROR) {
        fprintf(stderr, "there are compile time errors\n");
        exit(10);
    } else if (result == INTERPRET_RUNTIME_ERROR) {
        fprintf(stderr, "program terminated with runtime errors\n");
        exit(11);
    }

}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        runFile("./main.leaf");
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "usage: leaf <file_path>\n");
    }

    return 0;
}
