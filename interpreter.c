#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#include "interpreter.h"
#include "arena.h"
#include "environment.h"
#include "expression.h"
#include "object.h"
#include "statement.h"
#include "tokenizer.h"
#include "parser.h"
#include "value.h"


#define TOKEN_TYPE(expression)   ((expression)->meta.token.type)
#define TOKEN_LENGTH(expression) ((expression)->meta.token.length)
#define TOKEN_START(expression) ((expression)->meta.token.start)


typedef struct Interpreter {
    Environment *env;
} Interpreter;


static Interpreter interpreter;


static void initInterpreter() {
    interpreter.env = makeEnvironment();
}


static void freeInterpreter() {
    freeEnvironment(interpreter.env);
}


static Environment *env() { return interpreter.env; }


void error(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}


static Value evaluateExpression(const Expression *expression);


static Value evaluateComma(const Comma *expression) {
    Value result = evaluateExpression(expression->right);
    (void)evaluateExpression(expression->left);
    return result;
}


static Value evaluateTernary(const Ternary *expression) {
    Value condition = evaluateExpression(expression->condition);
    return isTruthy(condition) ? evaluateExpression(expression->first) : evaluateExpression(expression->second);
}


static Value evaluateBinary(const Binary *expression) {
    return performBinary(evaluateExpression(expression->left), evaluateExpression(expression->right), expression->meta.token);
}


static Value evaluateUnary(const Unary *expression) {
    return performUnary(evaluateExpression(expression->expression), expression->meta.token);
}


static Value evaluateTerminal(const Terminal *expression) {
    switch (TOKEN_TYPE(expression)) {
        case TOKEN_NUMBER: {
            return NUMBER_VALUE(strtod(TOKEN_START(expression), NULL));
        }
        case TOKEN_TRUE: {
            return BOOLEAN_VALUE(true);
        }
        case TOKEN_FALSE: {
            return BOOLEAN_VALUE(false);
        }
        case TOKEN_NIL: {
            return NIL_VALUE;
        }
        case TOKEN_STRING: {
            return OBJ_VALUE(makeString(TOKEN_START(expression), TOKEN_LENGTH(expression)));
        }
        default:
            break;
    }
    return NIL_VALUE;
}


static Value evaluateGroup(const Group *expression) {
    return evaluateExpression(expression->expression);
}


static Value evaluateExpression(const Expression *expression) {
    switch (expression->type) {
        case EXPRESSION_COMMA: return evaluateComma(AS_COMMA(expression));
        case EXPRESSION_TERNARY: return evaluateTernary(AS_TERNARY(expression));
        case EXPRESSION_BINARY: return evaluateBinary(AS_BINARY(expression));
        case EXPRESSION_UNARY: return evaluateUnary(AS_UNARY(expression));
        case EXPRESSION_GROUP: return evaluateGroup(AS_GROUP(expression));
        case EXPRESSION_TERMINAL: return evaluateTerminal(AS_TERMINAL(expression));
        default:
            break;
    }
    return NIL_VALUE;
}


static void executePut(const Put *statement) {
    Value value = evaluateExpression(statement->expression);
    printValue(value);
}


static void executePutln(const Putln *statement) {
    Value value = evaluateExpression(statement->expression);
    printValue(value);
    printf("\n");
}


static void executeExprStatement(const ExprStatement *statement) {
    (void)evaluateExpression(statement->expression);
}


static void executeStatement(const Statement *statement) {
    switch (statement->type) {
        case STATEMENT_PUT: return executePut(AS_PUT(statement));
        case STATEMENT_PUTLN: return executePutln(AS_PUTLN(statement));
        case STATEMENT_EXPRESSION: return executeExprStatement(AS_EXPR_STATEMENT(statement));
        default:
            break;
    }
}


static InterpretResult execute(const Statements *statements) {
    for (int i = 0; i < statements->count; i++) {
        executeStatement(statements->array[i]);
    }

    return INTERPRET_OK;
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
        freeTokens(&tokens);
        freeStatements(&statements);
        freeArena(&arena);
        return INTERPRET_COMPILE_ERROR;
    }
    freeTokens(&tokens);

    initInterpreter();
    InterpretResult result = execute(&statements);
    freeInterpreter();

    freeStatements(&statements);
    freeArena(&arena);
    return result;
}
