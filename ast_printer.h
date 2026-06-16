#ifndef leaf_ast_printer_h
#define leaf_ast_printer_h


#include "statement.h"


#define PRINT_AST(statements)   \
    (printAst(statements, 0))


void printAst(const Statements *statements, int idents);


#endif
