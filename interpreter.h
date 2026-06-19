#ifndef leaf_interpreter_h
#define leaf_interpreter_h


#include <stdlib.h>
#include <stdarg.h>


typedef enum InterpretResult {
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_OK,
} InterpretResult;


InterpretResult interpret(const char *source);

void printError(const char *format, ...);
void runtimeError(const char *message, int line);


#endif
