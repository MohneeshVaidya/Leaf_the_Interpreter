#include <stdio.h>


#include "interpreter.h"
#include "arena.h"
#include "tokenizer.h"
#include "parser.h"


void error(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}


InterpretResult interpret(const char *source) {
    Tokens tokens;
    if (!makeTokens(&tokens, source)) {
        Token *errorToken = &tokens.array[tokens.count - 2];
        error("compile_error: [at line %d], %.*s", errorToken->line, errorToken->length, errorToken->start);
        return INTERPRET_COMPILE_ERROR;
    }

    Arena arena;
    initArena(&arena);

    Statements statements;
    if (!parseStatements(&statements, &tokens, &arena)) {

    }
    freeTokens(&tokens);

    freeStatements(&statements);
    freeArena(&arena);
    return INTERPRET_OK;
}
