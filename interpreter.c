#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>


#include "interpreter.h"
#include "arena.h"
#include "environment.h"
#include "expression.h"
#include "object.h"
#include "statement.h"
#include "table.h"
#include "tokenizer.h"
#include "parser.h"
#include "value.h"


#define MAX_LOOP_DEPTH (1024)
#define MAX_CALL_DEPTH  (1024)


#define EXPRESSION_TOKEN(expression)   ((expression)->meta.token)


jmp_buf interpreterBuf;
jmp_buf breakBufs[MAX_LOOP_DEPTH];
jmp_buf continueBufs[MAX_LOOP_DEPTH];
jmp_buf functionBufs[MAX_CALL_DEPTH];


typedef struct Interpreter {
    Environment *env;
    Table strings;
    bool isLoop;
    bool isFunction;
    int loopIdx;
    int functionIdx;
    Value returnValue;
} Interpreter;


static Interpreter interpreter;


static void initInterpreter() {
    interpreter.env = makeEnv();
    initTable(&interpreter.strings);
    interpreter.isLoop = false;
    interpreter.isFunction = false;
    interpreter.loopIdx= -1;
    interpreter.functionIdx = -1;
    interpreter.returnValue = NIL_VALUE;
}


static void freeInterpreter() {
    freeEnv(interpreter.env);
    freeTable(&interpreter.strings);
    interpreter.isLoop = false;
    interpreter.isFunction = false;
    interpreter.loopIdx = -1;
    interpreter.functionIdx = -1;
    interpreter.returnValue = NIL_VALUE;
}


static Environment *env() { return interpreter.env; }
static Table *strings() { return &interpreter.strings; }
static bool isLoop() { return interpreter.isLoop; }
static bool isFunction() { return interpreter.isFunction; }
static int loopIdx() { return interpreter.loopIdx; }
static int functionIdx() { return interpreter.functionIdx; }
static Value returnValue() { return interpreter.returnValue; }


void printError(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
}


void runtimeError(const char *message, int line) {
    printError("runtime_error: [near line %d], %s", line, message);
    longjmp(interpreterBuf, 1);
}


static Value evaluateExpression(const Expression *expression);


static void executeStatement(const Statement *statement);
static void executeStatements(const Statements *statements);

static Value evaluateComma(const Comma *expression) {
    (void)evaluateExpression(expression->left);
    Value result = evaluateExpression(expression->right);
    return result;
}


static Value performCompoundAssign(ObjString *name, Value rightValue, Token token) {
    Value leftValue;
    if (!envGet(env(), name, &leftValue)) {
        runtimeError("left operand of compound assignment is undeclared", token.line);
    }

    TokenType type = token.type == TOKEN_PLUS_EQUAL ? TOKEN_PLUS : (
                        token.type == TOKEN_MINUS_EQUAL ? TOKEN_MINUS : (
                            token.type == TOKEN_SLASH_EQUAL ? TOKEN_SLASH : (
                                token.type ==  TOKEN_STAR_EQUAL ? TOKEN_STAR : (
                                    token.type == TOKEN_PERCENT_EQUAL ? TOKEN_PERCENT : TOKEN_STAR_STAR))));

    return performBinary(leftValue, rightValue, (Token){ .line = token.line, .type = type }, strings());
}


static Value evaluateAssign(const Assign *expression) {
    int line = EXPRESSION_TOKEN(expression).line;

    if (expression->left->token.type != TOKEN_IDENTIFIER) {
        runtimeError("'=' expects a name as left operand", line);
    }

    ObjString *name = internString(expression->left->token.start, expression->left->token.length, strings());
    Value value = evaluateExpression(expression->right);

    if (EXPRESSION_TOKEN(expression).type == TOKEN_EQUAL) {
        if (!envGet(env(), name, NULL)) {
            runtimeError("can not assign value to undeclared name", line);
        }
        envSet(env(), name, value);
    } else {
        value = performCompoundAssign(name, value, EXPRESSION_TOKEN(expression));
        envSet(env(), name, value);
    }
    return value;
}


static Value evaluateTernary(const Ternary *expression) {
    Value condition = evaluateExpression(expression->condition);
    return isTruthy(condition) ? evaluateExpression(expression->first) : evaluateExpression(expression->second);
}


static Value evaluateBinary(const Binary *expression) {
    Value left = evaluateExpression(expression->left);
    Value right = evaluateExpression(expression->right);
    return performBinary(left, right, expression->meta.token, strings());
}


static Value evaluateUnary(const Unary *expression) {
    return performUnary(evaluateExpression(expression->expression), expression->meta.token);
}


static Value callFunction(ObjFn *function, const Expressions *arguments, int line) {
    interpreter.returnValue = NIL_VALUE;

    if (arguments->count > MAX_PARAMETERS) {
        runtimeError("argument count exceeds 256", line);
    }

    if (arguments->count != function->arity) {
        runtimeError("argument count doesn't match the parameter count", line);
    }

    interpreter.functionIdx++;

    Environment *currentEnv = env();
    bool currentIsFunction = isFunction();
    interpreter.isFunction = true;

    Environment *env_ = makeEnv();
    for (int i = 0; i < function->arity; i++) {
        ObjString *name = internString(function->parameters[i].start, function->parameters[i].length, strings());
        envAdd(env_, name, evaluateExpression(arguments->array[i]));
    }
    env_->previous = function->closure;
    interpreter.env = env_;

    if (setjmp(functionBufs[functionIdx()]) == 0) {
        executeStatements(function->block->statements);
    }

    interpreter.isFunction = currentIsFunction;
    interpreter.env = currentEnv;

    interpreter.functionIdx--;
    return returnValue();
}


static Value evaluateCall(const Call *expression) {
    Value value = evaluateExpression(expression->object);
    if (!IS_OBJ_FN(value)) {
        runtimeError("only functions can be called", EXPRESSION_TOKEN(expression).line);
    }
    ObjFn *function = AS_OBJ_FN(value);
    return callFunction(function, expression->arguments, EXPRESSION_TOKEN(expression).line);
}


static Value getIdentifierValue(const Terminal *identifier) {
    ObjString *name = internString(EXPRESSION_TOKEN(identifier).start, EXPRESSION_TOKEN(identifier).length, strings());
    Value value;
    if (!envGet(env(), name, &value)) {
        runtimeError("trying to access undeclared name", identifier->meta.token.line);
    }
    return value;
}


static Value evaluateTerminal(const Terminal *expression) {
    switch (EXPRESSION_TOKEN(expression).type) {
        case TOKEN_NUMBER: {
            return NUMBER_VALUE(strtod(EXPRESSION_TOKEN(expression).start, NULL));
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
            return OBJ_VALUE(internString(EXPRESSION_TOKEN(expression).start, EXPRESSION_TOKEN(expression).length, strings()));
        }
        case TOKEN_IDENTIFIER: {
            return getIdentifierValue(expression);
        }
        default:
            break;
    }
    return NIL_VALUE;
}


static Value evaluateGroup(const Group *expression) {
    return evaluateExpression(expression->expression);
}


static Value evaluateFn(const Fn *expression) {
    ObjFn *objFn = makeObjFn((Parameter *)expression->parameters,
                             expression->arity,
                             expression->block,
                             env());
    return OBJ_VALUE(objFn);
}


static Value evaluateExpression(const Expression *expression) {
    if (expression == NULL) {
        return NIL_VALUE;
    }

    switch (expression->type) {
        case EXPRESSION_COMMA: return evaluateComma(AS_COMMA(expression));
        case EXPRESSION_ASSIGN: return evaluateAssign(AS_ASSIGN(expression));
        case EXPRESSION_TERNARY: return evaluateTernary(AS_TERNARY(expression));
        case EXPRESSION_BINARY: return evaluateBinary(AS_BINARY(expression));
        case EXPRESSION_UNARY: return evaluateUnary(AS_UNARY(expression));
        case EXPRESSION_CALL: return evaluateCall(AS_CALL(expression));
        case EXPRESSION_TERMINAL: return evaluateTerminal(AS_TERMINAL(expression));
        case EXPRESSION_GROUP: return evaluateGroup(AS_GROUP(expression));
        case EXPRESSION_FN: return evaluateFn(AS_FN(expression));
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


static void executeVar(const Var *statement) {
    ObjString *name = internString(statement->name.start, statement->name.length, strings());
    Value value = evaluateExpression(statement->expression);

    if (!envAdd(env(), name, value)) {
        runtimeError("redeclaring same name in same scope", statement->name.line);
    }
}


static void executeConst(const Const *statement) {
    printf("imp_message: dont use 'const' declaration for now\n");
}


static void beginScope() {
    Environment *env_ = makeEnv();
    env_->previous = env();
    interpreter.env = env_;
}


static void endScope() {
    Environment *env_ = env()->previous;
    freeEnv(env());
    interpreter.env = env_;
}


static void executeBlock(const Block *statement) {
    beginScope();
    executeStatements(statement->statements);
    endScope();
}


static bool executeConditionalBlock(const ConditionalBlock *block) {
    Value condition = NIL_VALUE;
    if (block->condition) {
        condition = evaluateExpression(block->condition);
        if (!isTruthy(condition)) {
            return false;
        }
    }

    executeBlock(block->block);

    return true;
}


static void executeIf(const If *statement) {
#define STATEMENT_AT(statement, index) ((statement)->statements->array[index])

    for (int i = 0; i < statement->statements->count; i++) {
        assert(IS_CONDITIONAL_BLOCK(STATEMENT_AT(statement, i)) && "If struct should have array of ConditionalBlock only");
        if (executeConditionalBlock(AS_CONDITIONAL_BLOCK(STATEMENT_AT(statement, i)))) break;
    }

#undef STATEMENT_AT
}


static void executeFor(const For *statement) {
    beginScope();
    bool prev = isLoop();
    interpreter.isLoop = true;
    interpreter.loopIdx++;

    if (setjmp(breakBufs[loopIdx()]) == 0) {
        if (statement->initialize) executeStatement(statement->initialize);

        for (;
            (!statement->condition) ? true : isTruthy(evaluateExpression(statement->condition));
        ) {
            if (setjmp(continueBufs[loopIdx()]) == 0) {
                executeBlock(statement->block);
            }
            if (statement->step) (void)evaluateExpression(statement->step);
        }
    }

    interpreter.loopIdx--;
    interpreter.isLoop = prev;
    endScope();
}


static void executeBreak(const Break *statement) {
    if (isLoop()) {
        longjmp(breakBufs[loopIdx()], 1);
    }
    runtimeError("'break' can only be used inside the context of loop", 0);
}


static void executeContinue(const Continue *statement) {
    if (isLoop()) {
        longjmp(continueBufs[loopIdx()], 1);
    }
    runtimeError("'continue' can only be used inside the context of loop", 0);
}


static void executeFnDeclaration(const FnDeclaration *statement) {
#define GET_FUNCTION(statement) ((statement)->function)

    ObjString *name = internString(statement->name.start, statement->name.length, strings());

    if (!envAdd(env(), name, evaluateFn(statement->function))) {
        runtimeError("can not redeclare a name in same scope", statement->name.line);
    }

#undef GET_FUNCTION
}


static void executeReturn(const Return *statement) {
    if (isFunction()) {
        interpreter.returnValue = evaluateExpression(statement->expression);
        longjmp(functionBufs[functionIdx()], 1);
    }
    runtimeError("'return' can only be used inside a function body", 0);
}


static void executeExprStatement(const ExprStatement *statement) {
    (void)evaluateExpression(statement->expression);
}


static void executeStatement(const Statement *statement) {
    switch (statement->type) {
        case STATEMENT_PUT: return executePut(AS_PUT(statement));
        case STATEMENT_PUTLN: return executePutln(AS_PUTLN(statement));
        case STATEMENT_VAR: return executeVar(AS_VAR(statement));
        case STATEMENT_BLOCK: return executeBlock(AS_BLOCK(statement));
        case STATEMENT_IF: return executeIf(AS_IF(statement));
        case STATEMENT_FOR: return executeFor(AS_FOR(statement));
        case STATEMENT_BREAK: return executeBreak(AS_BREAK(statement));
        case STATEMENT_CONTINUE: return executeContinue(AS_CONTINUE(statement));
        case STATEMENT_FN_DECLARATION: return executeFnDeclaration(AS_FN_DECLARATION(statement));
        case STATEMENT_RETURN: return executeReturn(AS_RETURN(statement));
        case STATEMENT_EXPRESSION: return executeExprStatement(AS_EXPR_STATEMENT(statement));
        default:
            break;
    }
}


static void executeStatements(const Statements *statements) {
    for (int i = 0; i < statements->count; i++) {
        executeStatement(statements->array[i]);
    }
}


static InterpretResult interpret_(const Statements *statements) {
    if (setjmp(interpreterBuf)) {
        return INTERPRET_RUNTIME_ERROR;
    }

    executeStatements(statements);

    return INTERPRET_OK;
}


static void freeThings(Tokens *tokens, Statements *statements, Arena *arena) {
    if (tokens) freeTokens(tokens);
    if (statements) freeStatements(statements);
    if (arena) freeArena(arena);
}


InterpretResult interpret(const char *source) {
    Tokens tokens;
    if (!makeTokens(&tokens, source)) {
        Token *errorToken = &tokens.array[tokens.count - 2];
        printError("compile_error: [at line %d], %.*s", errorToken->line, errorToken->length, errorToken->start);
        return INTERPRET_COMPILE_ERROR;
    }

    Arena arena;
    initArena(&arena);

    Statements statements;
    if (!parseStatements(&statements, &tokens, &arena)) {
        freeThings(&tokens, &statements, &arena);
        return INTERPRET_COMPILE_ERROR;
    }
    freeTokens(&tokens);

    initInterpreter();
    InterpretResult result = interpret_(&statements);
    freeInterpreter();

    freeThings(NULL, &statements, &arena);
    return result;
}
