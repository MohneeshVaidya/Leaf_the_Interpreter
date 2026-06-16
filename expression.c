#include <stddef.h>


#include "expression.h"
#include "arena.h"


#define MAKE_EXPRESSION(typeTag, token, arena, type) \
    ((type *)makeExpression(typeTag, token, arena, sizeof(type)))


static Expression *makeExpression(ExpressionType type, Token token, Arena *arena, size_t bytes) {
    Expression *expression = arenaAllocate(arena, bytes);
    expression->type = type;
    expression->token = token;
    return expression;
}


Comma *makeComma(Token token, Expression *left, Expression *right, Arena *arena) {
    Comma *expression = MAKE_EXPRESSION(EXPRESSION_COMMA, token, arena, Comma);
    expression->left = left;
    expression->right = right;
    return expression;
}


Assign *makeAssign(Token token, Expression *left, Expression *right, Arena *arena) {
    Assign *expression = MAKE_EXPRESSION(EXPRESSION_ASSIGN, token, arena, Assign);
    expression->left = left;
    expression->right = right;
    return expression;
}


Ternary *makeTernary(Token token, Expression *condition, Expression *first, Expression *second, Arena *arena) {
    Ternary *expression = MAKE_EXPRESSION(EXPRESSION_TERNARY, token, arena, Ternary);
    expression->condition = condition;
    expression->first = first;
    expression->second = second;
    return expression;
}


Binary *makeBinary(Token token, Expression *left, Expression *right, Arena *arena) {
    Binary *expression = MAKE_EXPRESSION(EXPRESSION_BINARY, token, arena, Binary);
    expression->left = left;
    expression->right = right;
    return expression;
}


Unary *makeUnary(Token token, Expression *expression, Arena *arena) {
    Unary *expression_ = MAKE_EXPRESSION(EXPRESSION_UNARY, token, arena, Unary);
    expression_->expression = expression;
    return expression_;
}


Call *makeCall(Token token, Expression *callee, Expressions *arguments, Arena *arena) {
    Call *expression = MAKE_EXPRESSION(EXPRESSION_CALL, token, arena, Call);
    expression->object = callee;
    expression->arguments = arguments;
    return expression;
}


Get *makeGet(Token token, Expression *object, Terminal *field, Arena *arena) {
    Get *expression = MAKE_EXPRESSION(EXPRESSION_GET, token, arena, Get);
    expression->object = object;
    expression->field = field;
    return expression;
}


Set *makeSet(Token token, Expression *object, Terminal *field, Expression *value, Arena *arena) {
    Set *expression = MAKE_EXPRESSION(EXPRESSION_SET, token, arena, Set);
    expression->object = object;
    expression->field = field;
    expression->value = value;
    return expression;
}


Group *makeGroup(Token token, Expression *expression, Arena *arena) {
    Group *expression_ = MAKE_EXPRESSION(EXPRESSION_GROUP, token, arena, Group);
    expression_->expression = expression;
    return expression_;
}

Terminal *makeTerminal(Token token, Arena *arena) {
    Terminal *expression = MAKE_EXPRESSION(EXPRESSION_TERMINAL, token, arena, Terminal);
    return expression;
}


Fn *makeFn(Token token, Arena *arena) {
    Fn *expression = MAKE_EXPRESSION(EXPRESSION_FN, token, arena, Fn);
    expression->arity = 0;
    expression->block = NULL;
    return expression;
}


Struct *makeStruct(Token token, struct Block *block, Arena *arena) {
    Struct *expression = MAKE_EXPRESSION(EXPRESSION_STRUCT, token, arena , Struct);
    expression->block = block;
    return expression;
}


Expressions *makeExpressions(Arena *arena) {
    Expressions *expressions = ARENA_ALLOCATE(arena, Expressions);
    expressions->capacity = 0;
    expressions->count = 0;
    expressions->array = NULL;
    return expressions;
}
