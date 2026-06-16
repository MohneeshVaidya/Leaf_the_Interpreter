#include "statement.h"
#include "arena.h"


#define MAKE_STATEMENT(typeTag, arena, type)  \
    ((type *)makeStatement(typeTag, arena, sizeof(type)))


static Statement *makeStatement(StatementType type, Arena *arena, size_t bytes) {
    Statement *statement = arenaAllocate(arena, bytes);
    statement->type = type;
    return statement;
}


ExprStatement *makeExprStatement(Expression *expression, Arena *arena) {
    ExprStatement *statement = MAKE_STATEMENT(STATEMENT_EXPRESSION, arena, ExprStatement);
    statement->expression = expression;
    return statement;
}


Put *makePut(Expression *expression, Arena *arena) {
    Put *statement = MAKE_STATEMENT(STATEMENT_PUT, arena, Put);
    statement->expression = expression;
    return statement;
}


Putln *makePutln(Expression *expression, Arena *arena) {
    Putln *statement = MAKE_STATEMENT(STATEMENT_PUTLN, arena, Putln);
    statement->expression = expression;
    return statement;
}


Var *makeVar(Token name, Expression *expression, Arena *arena) {
    Var *statement = MAKE_STATEMENT(STATEMENT_VAR, arena, Var);
    statement->name = name;
    statement->expression = expression;
    return statement;
}


Const *makeConst(Token name, Expression *expression, Arena *arena) {
    Const *statement = MAKE_STATEMENT(STATEMENT_CONST, arena, Const);
    statement->name = name;
    statement->expression = expression;
    return statement;
}


Block *makeBlock(Statements *statements, Arena *arena) {
    Block *statement = MAKE_STATEMENT(STATEMENT_BLOCK, arena, Block);
    statement->statements = statements;
    return statement;
}


ConditionalBlock *makeConditionalBlock(Expression *condition, Block *block, Arena *arena) {
    ConditionalBlock *statement = MAKE_STATEMENT(STATEMENT_CONDITIONAL_BLOCK, arena, ConditionalBlock);
    statement->condition = condition;
    statement->block = block;
    return statement;
}


If *makeIf(Statements *statements, Arena *arena) {
    If *statement = MAKE_STATEMENT(STATEMENT_IF, arena, If);
    statement->statements = statements;
    return statement;
}


For *makeFor(Statement *initialize, Expression *condition, Expression *step, Block *block, Arena *arena) {
    For *statement = MAKE_STATEMENT(STATEMENT_FOR, arena, For);
    statement->initialize = initialize;
    statement->condition = condition;
    statement->step = step;
    statement->block = block;
    return statement;
}


Break *makeBreak(Arena *arena) {
    return MAKE_STATEMENT(STATEMENT_BREAK, arena, Break);
}


Continue *makeContinue(Arena *arena) {
    return MAKE_STATEMENT(STATEMENT_CONTINUE, arena, Continue);
}


FnDeclaration *makeFnDeclaration(Token name, Fn *function, Arena *arena) {
    FnDeclaration *statement = MAKE_STATEMENT(STATEMENT_FN_DECLARATION, arena, FnDeclaration);
    statement->name = name;
    statement->function = function;
    return statement;
}


Return *makeReturn(Expression *expression, Arena *arena) {
    Return *statement = MAKE_STATEMENT(STATEMENT_RETURN, arena, Return);
    statement->expression = expression;
    return statement;
}


Statements *makeStatements(Arena *arena) {
    Statements *statements = ARENA_ALLOCATE(arena, Statements);
    statements->capacity = 0;
    statements->count = 0;
    statements->array = NULL;
    return statements;
}
