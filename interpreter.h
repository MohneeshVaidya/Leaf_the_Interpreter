#ifndef leaf_interpreter_h
#define leaf_interpreter_h


#include <stdarg.h>


typedef enum InterpretResult {
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_OK,
} InterpretResult;


InterpretResult interpret(const char *source);
void error(const char *format, ...);


#endif
