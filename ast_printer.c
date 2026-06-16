#include <stdio.h>


#include "ast_printer.h"
#include "expression.h"
#include "statement.h"


#define TOKEN_LENGTH(expression)    ((expression)->meta.token.length)
#define TOKEN_START(expression)     ((expression)->meta.token.start)
#define TOKEN_TYPE(expression)      ((expression)->meta.token.type)


#define IDENT_SIZE 4


#define PRINT_INDENTS(idents)           \
    for (int i = 0; i < idents; i++) {  \
        printf(" ");                    \
    }


static void printExpression(const Expression *expression, int idents);
static void printBlock(const Block *block, int idents);


static void printComma(const Comma *expression, int idents) {
    printf("( %.*s ", TOKEN_LENGTH(expression), TOKEN_START(expression));
    printExpression(expression->left, idents);
    printExpression(expression->right, idents);
    printf(") ");
}


static void printAssign(const Assign *expression, int idents) {
    printf("( %.*s ", TOKEN_LENGTH(expression), TOKEN_START(expression));
    printExpression(expression->left, idents);
    printExpression(expression->right, idents);
    printf(") ");
}


static void printTernary(const Ternary *expression, int idents) {
    printf("( ");
    printExpression(expression->condition, idents);
    printf("%.*s ", TOKEN_LENGTH(expression), TOKEN_START(expression));
    printExpression(expression->first, idents);
    printf(": ");
    printExpression(expression->second, idents);
    printf(") ");
}


static void printBinary(const Binary *expression, int idents) {
    printf("( %.*s ", TOKEN_LENGTH(expression), TOKEN_START(expression));
    printExpression(expression->left, idents);
    printExpression(expression->right, idents);
    printf(") ");
}


static void printUnary(const Unary *expression, int idents) {
    printf("( %.*s ", TOKEN_LENGTH(expression), TOKEN_START(expression));
    printExpression(expression->expression, idents);
    printf(") ");
}


static void printArguments(Expressions *arguments, int idents) {
    if (arguments == NULL) {
        return;
    }
    for (int i = 0; i < arguments->count; i++) {
        printExpression(arguments->array[i], idents);
        if (i < arguments->count - 1) {
            printf(", ");
        }
    }
}


static void printCall(const Call *expression, int idents) {
    printExpression(expression->callee, idents);
    printf("( ");
    printArguments(expression->arguments, idents);
    printf(") ");
}


static void printGet(const Get *expression, int idents) {

}


static void printSet(const Set *expression, int idents) {

}


static void printGroup(const Group *expression, int idents) {
    printf("( group ");
    printExpression(expression->expression, idents);
    printf(") ");
}


static void printTerminal(const Terminal *expression) {
    bool isString = TOKEN_TYPE(expression) == TOKEN_STRING;

    printf("%s%.*s%s ", isString ? "\"" : "", TOKEN_LENGTH(expression), TOKEN_START(expression), isString ? "\"" : "");
}


static void printParameters(const Parameter *parameters, int arity) {
    printf("(");
    for (int i = 0; i < arity; i++) {
        printf("%.*s", parameters[i].length, parameters[i].start);
        if (i < arity - 1) printf(", ");
    }
    printf(") ");
}


static void printFn(const Fn *expression, int idents) {
    printf("fn ");
    printParameters(expression->parameters, expression->arity);
    printBlock(expression->block, idents);
}


static void printExpression(const Expression *expression, int idents) {
    if (expression == NULL) {
        printf("nil ");
        return;
    }

    switch (expression->type) {
        case EXPRESSION_COMMA: return printComma(AS_COMMA(expression), idents);
        case EXPRESSION_ASSIGN: return printAssign(AS_ASSIGN(expression), idents);
        case EXPRESSION_TERNARY: return printTernary(AS_TERNARY(expression), idents);
        case EXPRESSION_BINARY: return printBinary(AS_BINARY(expression), idents);
        case EXPRESSION_UNARY: return printUnary(AS_UNARY(expression), idents);
        case EXPRESSION_CALL: return printCall(AS_CALL(expression), idents);
        case EXPRESSION_GET: return printGet(AS_GET(expression), idents);
        case EXPRESSION_SET: return printSet(AS_SET(expression), idents);
        case EXPRESSION_GROUP: return printGroup(AS_GROUP(expression), idents);
        case EXPRESSION_TERMINAL: return printTerminal(AS_TERMINAL(expression));
        case EXPRESSION_FN: return printFn(AS_FN(expression), idents + IDENT_SIZE);
        default:
            break;
    }
}


static void printStatement(const Statement *statement, int idents);


static void printExprStatement(const ExprStatement *statement, int idents) {
    printExpression(statement->expression, idents);
    printf("\b; ");
}


static void printPut(const Put *statement, int idents) {
    printf("put ");
    printExpression(statement->expression, idents);
    printf("\b; ");
}


static void printPutln(const Putln *statement, int idents) {
    printf("putln ");
    printExpression(statement->expression, idents);
    printf("\b; ");
}


static void printVar(const Var *statement, int idents) {
    printf("var %.*s = ", statement->name.length, statement->name.start);
    printExpression(statement->expression, idents);
    printf("\b; ");
}


static void printConst(const Const *statement, int idents) {
    printf("const %.*s = ", statement->name.length, statement->name.start);
    printExpression(statement->expression, idents);
    printf("\b; ");
}


static void printBlock(const Block *block, int idents) {
    printf("{\n\n");
    printAst(block->statements, idents);
    PRINT_INDENTS(idents - IDENT_SIZE);
    printf("} ");
}


static void printConditionalBlock(const ConditionalBlock *statement, int idents) {
    printExpression(statement->condition, idents);
    printBlock(statement->block, idents);
}


static void printIf(const If *statement, int idents) {
#define GET_COUNT(statement)        (statement->statements->count)
#define GET_BLOCK(statement, index) (AS_CONDITIONAL_BLOCK(statement->statements->array[index]))

    printf("if ");

    printConditionalBlock(GET_BLOCK(statement, 0), idents);

    for (int i = 1; i < GET_COUNT(statement); i++) {
        printf("elseif ");
        printConditionalBlock(GET_BLOCK(statement, i), idents);
    }

#undef GET_COUNT
#undef GET_BLOCK
}


static void printFor(const For *statement, int idents) {
    printf("for ");
    printStatement(statement->initialize, 0);
    printExpression(statement->condition, idents);
    printf("\b; ");
    printExpression(statement->step, idents);
    printBlock(statement->block, idents);
}


static void printBreak() {
    printf("break; ");
}


static void printContinue() {
    printf("continue; ");
}


static void printFnDeclaration(const FnDeclaration *statement, int idents) {
    printf("var %.*s = ", statement->name.length, statement->name.start);
    printFn(statement->function, idents);
    printf("\b;");
}


static void printReturn(Return *statement, int idents) {
    printf("return ");
    printExpression(statement->expression, idents);
    printf("\b;");
}


static void printStatement(const Statement *statement, int idents) {
    PRINT_INDENTS(idents);

    if (statement == NULL) {
        printf("nil; ");
        return;
    }

    switch (statement->type) {
        case STATEMENT_EXPRESSION: return printExprStatement(AS_EXPR_STATEMENT(statement), idents);
        case STATEMENT_PUT: return printPut(AS_PUT(statement), idents);
        case STATEMENT_PUTLN: return printPutln(AS_PUTLN(statement), idents);
        case STATEMENT_VAR: return printVar(AS_VAR(statement), idents);
        case STATEMENT_CONST: return printConst(AS_CONST(statement), idents);
        case STATEMENT_BLOCK: return printBlock(AS_BLOCK(statement), idents + IDENT_SIZE);
        case STATEMENT_IF: return printIf(AS_IF(statement), idents + IDENT_SIZE);
        case STATEMENT_FOR: return printFor(AS_FOR(statement), idents + IDENT_SIZE);
        case STATEMENT_BREAK: return printBreak();
        case STATEMENT_CONTINUE: return printContinue();
        case STATEMENT_FN_DECLARATION: return printFnDeclaration(AS_FN_DECLARATION(statement), idents + IDENT_SIZE);
        case STATEMENT_RETURN: return printReturn(AS_RETURN(statement), idents);
        default:
            break;
    }
}


void printAst(const Statements *statements, int idents) {
    for (int i = 0; i < statements->count; i++) {
        printStatement(statements->array[i], idents);
        printf("\n\n");
    }
}
