#ifndef leaf_statement_h
#define leaf_statement_h


#include <stddef.h>


#include "tokenizer.h"
#include "expression.h"
#include "arena.h"
#include "memory.h"


#define AS_STATEMENT(statement) ((Statement *)statement)


#define IS_EXPRESSION(statement)        ((statement)->type == STATEMENT_EXPRESSION)
#define IS_PUT(statement)               ((statement)->type == STATEMENT_PUT)
#define IS_PUTLN(statement)             ((statement)->type == STATEMENT_PUTLN)
#define IS_VAR(statement)               ((statement)->type == STATEMENT_VAR)
#define IS_CONST(statement)             ((statement)->type == STATEMENT_CONST)
#define IS_BLOCK(statement)             ((statement)->type == STATEMENT_BLOCK)
#define IS_CONDITIONAL_BLOCK(statement) ((statement)->type == STATEMENT_CONDITIONAL_BLOCK)
#define IS_IF(statement)                ((statement)->type == STATEMENT_IF)
#define IS_FOR(statement)               ((statement)->type == STATEMENT_FOR)
#define IS_BREAK(statement)             ((statement)->type == STATEMENT_BREAK)
#define IS_CONTINUE(statement)          ((statement)->type == STATEMENT_CONTINUE)
#define IS_FN_DECLARATION(statement)    ((statement)->type == STATEMENT_FN_DECLARATION)
#define IS_RETURN(statement)            ((statement)->type == STATEMENT_RETURN)
#define IS_STRUCT(statement)            ((statement)->type == STATEMENT_STRUCT)


#define AS_EXPR_STATEMENT(statement)    ((ExprStatement *)statement)
#define AS_PUT(statement)               ((Put *)statement)
#define AS_PUTLN(statement)             ((Putln *)statement)
#define AS_VAR(statement)               ((Var *)statement)
#define AS_CONST(statement)             ((Const *)statement)
#define AS_BLOCK(statement)             ((Block *)statement)
#define AS_CONDITIONAL_BLOCK(statement) ((ConditionalBlock *)statement)
#define AS_IF(statement)                ((If *)statement)
#define AS_FOR(statement)               ((For *)statement)
#define AS_BREAK(statement)             ((Break *)statement)
#define AS_CONTINUE(statement)          ((Continue *)statement)
#define AS_FN_DECLARATION(statement)    ((FnDeclaration *)statement)
#define AS_RETURN(statement)            ((Return *)statement)


typedef enum StatementType {
    STATEMENT_EXPRESSION,
    STATEMENT_PUT,
    STATEMENT_PUTLN,
    STATEMENT_VAR,
    STATEMENT_CONST,
    STATEMENT_BLOCK,
    STATEMENT_CONDITIONAL_BLOCK,
    STATEMENT_IF,
    STATEMENT_FOR,
    STATEMENT_BREAK,
    STATEMENT_CONTINUE,
    STATEMENT_FN_DECLARATION,
    STATEMENT_RETURN,
    STATEMENT_STRUCT,
} StatementType;


typedef struct Statements Statements;


typedef struct Statement {
    StatementType type;
} Statement;


typedef struct ExprStatement {
    Statement meta;
    Expression *expression;
} ExprStatement;


typedef struct Put {
    Statement meta;
    Expression *expression;
} Put;


typedef struct Putln {
    Statement meta;
    Expression *expression;
} Putln;


typedef struct Var {
    Statement meta;
    Token name;
    Expression *expression;
} Var;


typedef struct Const {
    Statement meta;
    Token name;
    Expression *expression;
} Const;


typedef struct Block {
    Statement meta;
    Statements *statements;
} Block;


typedef struct ConditionalBlock {
    Statement meta;
    Expression *condition;
    Block *block;
} ConditionalBlock;


typedef struct If{
    Statement meta;
    Statements *statements; // Only conditional blocks here
} If;


typedef struct For {
    Statement meta;
    Statement *initialize;
    Expression *condition;
    Expression *step;
    Block *block;
} For;


typedef struct Break {
    Statement meta;
} Break;


typedef struct Continue {
    Statement meta;
} Continue;


typedef struct FnDeclaration {
    Statement meta;
    Token name;
    Fn *function;
} FnDeclaration;


typedef struct Return {
    Statement meta;
    Expression *expression;
} Return;


ExprStatement *makeExprStatement(Expression *expression, Arena *arena);
Put *makePut(Expression *expression, Arena *arena);
Putln *makePutln(Expression *expression, Arena *arena);
Var *makeVar(Token name, Expression *expression, Arena *arena);
Const *makeConst(Token name, Expression *expression, Arena *arena);
Block *makeBlock(Statements *statements, Arena *arena);
ConditionalBlock *makeConditionalBlock(Expression *condition, Block *block, Arena *arena);
If *makeIf(Statements *statements, Arena *arena);
For *makeFor(Statement *initialize, Expression *condition, Expression *step, Block *block, Arena *arena);
Break *makeBreak(Arena *arena);
Continue *makeContinue(Arena *arena);
FnDeclaration *makeFnDeclaration(Token name, Fn *function, Arena *arena);
Return *makeReturn(Expression *expression, Arena *arena);


struct Statements {
    int capacity;
    int count;
    Statement **array;
};


Statements *makeStatements(Arena *arena);


#endif
