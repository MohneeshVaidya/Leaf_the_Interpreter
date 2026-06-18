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
void error(const char *format, ...);

static inline void runtimeError(const char *message, int line) {
    error("runtime_error: [near line %d], %s", line, message);
    exit(1);
}


#endif
