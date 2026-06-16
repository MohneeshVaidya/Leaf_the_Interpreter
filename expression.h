#ifndef leaf_expression_h
#define leaf_expression_h


#include <stddef.h>


#include "arena.h"
#include "tokenizer.h"


#define MAX_PARAMETERS 255


#define AS_EXPRESSION(expression)   ((Expression *)expression)


#define IS_COMMA(expression)        ((expression)->type == EXPRESSION_COMMA)
#define IS_ASSIGN(expression)       ((expression)->type == EXPRESSION_ASSIGN)
#define IS_TERNARY(expression)      ((expression)->type == EXPRESSION_TERNARY)
#define IS_BINARY(expression)       ((expression)->type == EXPRESSION_BINARY)
#define IS_UNARY(expression)        ((expression)->type == EXPRESSION_UNARY)
#define IS_CALL(expression)         ((expression)->type == EXPRESSION_CALL)
#define IS_GET(expression)          ((expression)->type == EXPRESSION_GET)
#define IS_SET(expression)          ((expression)->type == EXPRESSION_SET)
#define IS_GROUP(expression)        ((expression)->type == EXPRESSION_GROUP)
#define IS_TERMINAL(expression)     ((expression)->type == EXPRESSION_TERMINAL)
#define IS_FN(expression)           ((expression)->type == EXPRESSION_FN)
#define IS_STRUCT(expression)       ((expression)->type == EXPRESSION_STRUCT)


#define AS_COMMA(expression)        ((Comma *)expression)
#define AS_ASSIGN(expression)       ((Assign *)expression)
#define AS_TERNARY(expression)      ((Ternary *)expression)
#define AS_BINARY(expression)       ((Binary *)expression)
#define AS_UNARY(expression)        ((Unary *)expression)
#define AS_CALL(expression)         ((Call *)expression)
#define AS_GET(expression)          ((Get *)expression)
#define AS_SET(expression)          ((Set *)expression)
#define AS_GROUP(expression)        ((Group *)expression)
#define AS_TERMINAL(expression)     ((Terminal *)expression)
#define AS_FN(expression)           ((Fn *)expression)
#define AS_STRUCT(expression)       ((Struct *)expression)


typedef enum ExpressionType {
    EXPRESSION_COMMA,
    EXPRESSION_ASSIGN,
    EXPRESSION_TERNARY,
    EXPRESSION_BINARY,
    EXPRESSION_UNARY,
    EXPRESSION_CALL,
    EXPRESSION_GET,
    EXPRESSION_SET,
    EXPRESSION_GROUP,
    EXPRESSION_TERMINAL,
    EXPRESSION_FN,
    EXPRESSION_STRUCT,
} ExpressionType;


typedef struct Expressions Expressions;
typedef struct Terminal Terminal;


typedef struct Expression {
    ExpressionType type;
    Token token;
} Expression;


typedef struct Comma {
    Expression meta;
    Expression *left;
    Expression *right;
} Comma;


typedef struct Assign {
    Expression meta;
    Expression *left;
    Expression *right;
} Assign;


typedef struct Ternary {
    Expression meta;
    Expression *condition;
    Expression *first;
    Expression *second;
} Ternary;


typedef struct Binary {
    Expression meta;
    Expression *left;
    Expression *right;
} Binary;


typedef struct Unary {
    Expression meta;
    Expression *expression;
} Unary;


typedef struct Call {
    Expression meta;
    Expression *object;
    Expressions *arguments;
} Call;


typedef struct Get {
    Expression meta;
    Expression *object;
    Terminal *field;
} Get;


typedef struct Set {
    Expression meta;
    Expression *object;
    Terminal *field;
    Expression *value;
} Set;


typedef struct Group {
    Expression meta;
    Expression *expression;
} Group;


struct Terminal {
    Expression meta;
};


typedef struct Parameter {
    const char *start;
    int length;
} Parameter;


struct Block;
typedef struct Fn {
    Expression meta;
    Parameter parameters[MAX_PARAMETERS];
    int arity;
    struct Block *block;
} Fn;


typedef struct Struct {
    Expression meta;
    struct Block *block;
} Struct;


Comma *makeComma(Token token, Expression *left, Expression *right, Arena *arena);
Assign *makeAssign(Token token, Expression *left, Expression *right, Arena *arena);
Ternary *makeTernary(Token token, Expression *condition, Expression *first, Expression *second, Arena *arena);
Binary *makeBinary(Token token, Expression *left, Expression *right, Arena *arena);
Unary *makeUnary(Token token, Expression *expression, Arena *arena);
Call *makeCall(Token token, Expression *object, Expressions *arguments, Arena *arena);
Get *makeGet(Token token, Expression *object, Terminal *field, Arena *arena);
Set *makeSet(Token token, Expression *object, Terminal *field, Expression *value, Arena *arena);
Group *makeGroup(Token token, Expression *expression, Arena *arena);
Terminal *makeTerminal(Token token, Arena *arena);
Fn *makeFn(Token token, Arena *arena);
Struct *makeStruct(Token token, struct Block *block, Arena *arena);


struct Expressions {
    int capacity;
    int count;
    Expression **array;
};


Expressions *makeExpressions(Arena *arena);


#endif
