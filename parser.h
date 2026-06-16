#ifndef leaf_parser_h
#define leaf_parser_h


#include "tokenizer.h"
#include "statement.h"


bool parseStatements(Statements *statements, const Tokens *tokens, Arena *arena);
void freeStatements(Statements *statements);


#endif
