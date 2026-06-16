#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>


#include "parser.h"
#include "expression.h"
#include "memory.h"
#include "statement.h"
#include "interpreter.h"
#include "tokenizer.h"


#define DEBUG_PRINT_AST


#ifdef DEBUG_PRINT_AST
#include "ast_printer.h"
#endif


static jmp_buf buf;


typedef struct Parser {
    Arena *arena;
    Statements *statements;
    const Token *tokens;
    const Token *current;
    bool hadErrors;
} Parser;


static Parser parser;


void initParser(Statements *statements, const Token *tokens, Arena *arena) {
    parser.arena = arena;
    parser.statements = statements;
    parser.tokens = tokens;
    parser.current = parser.tokens;
    parser.hadErrors = false;
}


void freeParser() {
    parser.arena = NULL;
    parser.statements = NULL;
    parser.tokens = parser.current = NULL;
    parser.hadErrors = false;
}


static inline Arena *arena() { return parser.arena; }
static inline Statements *statements() { return parser.statements; }
static inline const Token *tokens() { return parser.tokens; }
static inline const Token *current() { return parser.current; }
static inline const Token *previous() { return (current() == tokens()) ? current() : (current() - 1); }
static inline bool hadErrors() { return parser.hadErrors; }


static void parseError(const char *message) {
    parser.hadErrors = true;
    int line = previous()->line;
    error("parse_error: [near line %d], %s", line, message);

    longjmp(buf, 1);
}


static inline bool isAtEnd() {
    return current()->type == TOKEN_EOF;
}


static inline const Token *forward() {
    if (current()->type != TOKEN_EOF) return parser.current++;
    return current();
}


static inline const Token *backward() {
    if (current() == tokens()) return parser.current;
    return parser.current--;
}


static const Token *consume(TokenType type, const char *message) {
    if (current()->type == type) {
        return forward();
    }

    parseError(message);
    return NULL;
}


static const Token *match(TokenType type) {
    if (current()->type == type) {
        return forward();
    }
    return NULL;
}


static const Token *matchAny(TokenType *types, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (current()->type == *(types + i)) {
            return forward();
        }
    }
    return NULL;
}


static const Token *check(TokenType type) {
    if (current()->type == type) return current();
    return NULL;
}


static Expression *parseComma();
static Expression *parseAssign();
static Expression *parseTernary();
static Expression *parseOr();
static Expression *parseAnd();
static Expression *parseEqual();
static Expression *parseComparision();
static Expression *parseTerm();
static Expression *parseFactor();
static Expression *parsePower();
static Expression *parseUnary();
static Expression *parseCall();
static Expression *parseTerminal();


static Statement *parsePut() {
    Expression *expression = parseComma();
    consume(TOKEN_SEMICOLON, "expect ';' after statement");
    return AS_STATEMENT(makePut(expression, arena()));
}


static Statement *parsePutln() {
    Expression *expression = parseComma();
    consume(TOKEN_SEMICOLON, "expect ';' after statement");
    return AS_STATEMENT(makePutln(expression, arena()));
}


static Statement *parseVar() {
    const Token *name = consume(TOKEN_IDENTIFIER, "expect a name after 'var' keyword");

    if (match(TOKEN_SEMICOLON)) {
        return AS_STATEMENT(makeVar(*name, NULL, arena()));
    }
    if (match(TOKEN_EQUAL)) {
        Expression *expression = parseComma();
        consume(TOKEN_SEMICOLON, "expect ';' after statement");
        return AS_STATEMENT(makeVar(*name, expression, arena()));
    }

    parseError("semicolon and optional initialization expression are missing");
    return NULL;
}


static Statement *parseConst() {
    const Token *name = consume(TOKEN_IDENTIFIER, "expect a name after 'const' keyword");

    if (consume(TOKEN_EQUAL, "expect explicit initialization '= expression' in const declaration")) {
        Expression *expression = parseComma();
        consume(TOKEN_SEMICOLON, "expect ';' after statement");
        return AS_STATEMENT(makeConst(*name, expression, arena()));
    }

    return NULL;
}


static void appendStatement(Statements *statements, Statement *statement);
static Statement *parseStatement();


static Statement *parseBlock() {
    Statements *statements = makeStatements(arena());

    for (; !isAtEnd() && current()->type != TOKEN_RIGHT_BRACE;) {
        appendStatement(statements, parseStatement());
    }
    if (isAtEnd()) {
        parseError("expect '}' for terminating the block");
    }
    forward();

    return AS_STATEMENT(makeBlock(statements, arena()));
}


static inline Statement *parseConditionalBlock() {
    Expression *condition = parseComma();
    consume(TOKEN_LEFT_BRACE, "expect '{' after if condition");
    Block *block = AS_BLOCK(parseBlock());
    return AS_STATEMENT(makeConditionalBlock(condition, block, arena()));
}


static Statement *parseIf() {
    Statements *statements = makeStatements(arena());

    appendStatement(statements, parseConditionalBlock());
    for (;match(TOKEN_ELSEIF);) {
        appendStatement(statements, parseConditionalBlock());
    }
    if (match(TOKEN_ELSE)) {
        forward();
        appendStatement(statements, AS_STATEMENT(makeConditionalBlock(NULL, AS_BLOCK(parseBlock()), arena())));
    }

    return AS_STATEMENT(makeIf(statements, arena()));
}


static Statement *parseRemainingFor(Statement *initialize) {
    Expression *condition = NULL;
    if (!match(TOKEN_SEMICOLON)) {
        condition = parseComma();
        consume(TOKEN_SEMICOLON, "expect ';' after 'for' condition");
    }

    Expression *step = NULL;
    if (!match(TOKEN_LEFT_BRACE)) {
        step = parseComma();
        consume(TOKEN_LEFT_BRACE, "expect '{' in 'for' statement");
    }
    return AS_STATEMENT(makeFor(initialize, condition, step, AS_BLOCK(parseBlock()), arena()));
}


static Statement *parseFor() {
    if (match(TOKEN_LEFT_BRACE)) {
        return AS_STATEMENT(makeFor(NULL, NULL, NULL, AS_BLOCK(parseBlock()), arena()));    // infinite for -> for { statements* }
    }

    const Token *token = NULL;
    if ((token = matchAny((TokenType[]){ TOKEN_VAR, TOKEN_CONST }, 2))) {
        Statement *initialize = NULL;
        if (token->type == TOKEN_VAR) {
            initialize = parseVar();
        } else {
            initialize = parseConst();
        }
        return parseRemainingFor(initialize);
    }

    if (match(TOKEN_SEMICOLON)) {
        return parseRemainingFor(NULL);
    } else {
        Expression *expression = parseComma();
        if (match(TOKEN_LEFT_BRACE)) {
            return AS_STATEMENT(makeFor(NULL, expression, NULL, AS_BLOCK(parseBlock()), arena()));  // while like for -> for condition { statements* }
        }
        consume(TOKEN_SEMICOLON, "expect ';' after initialization expression in 'for' statement");
        Statement *initialize = AS_STATEMENT(makeExprStatement(expression, arena()));
        return AS_STATEMENT(parseRemainingFor(initialize));
    }
}


static inline Statement *parseBreak() {
    consume(TOKEN_SEMICOLON, "expect ';' after statement");
    return AS_STATEMENT(makeBreak(arena()));
}


static inline Statement *parseContinue() {
    consume(TOKEN_SEMICOLON, "expect ';' after statement");
    return AS_STATEMENT(makeContinue(arena()));
}


static void parseFnParameters(Fn *function) {
    if (!match(TOKEN_RIGHT_PAREN)) {
        for (;;) {
            if (function->arity == MAX_PARAMETERS) {
                parseError("parameters count exceed 255");
            }

            const Token *name = consume(TOKEN_IDENTIFIER, "expect a name");
            function->parameters[function->arity++] = (Parameter){ .start = name->start, .length = name->length };

            if (match(TOKEN_RIGHT_PAREN)) break;
            consume(TOKEN_COMMA, "expect ',' between parameters");
        }
    }
}


static Fn *parseFnExpression() {
    const Token *token = consume(TOKEN_LEFT_PAREN, "expect '(' after function's name");

    Fn *function = makeFn(*token, arena());
    parseFnParameters(function);

    consume(TOKEN_LEFT_BRACE, "expect '{' after parameters list");

    function->block = AS_BLOCK(parseBlock());
    return function;
}


static Statement *parseFn() {
    const Token *name = match(TOKEN_IDENTIFIER);
    if (!name && check(TOKEN_LEFT_PAREN)) {
        backward();
        Statement *statement = AS_STATEMENT(makeExprStatement(AS_EXPRESSION(parseCall()), arena()));
        consume(TOKEN_SEMICOLON, "expect ';' after statement");
        return statement;
    } else if (!name) {
        parseError("expect either a function declaration or expression");
    }
    return AS_STATEMENT(makeFnDeclaration(*name, parseFnExpression(), arena()));
}


static Statement *parseReturn() {
    if (match(TOKEN_SEMICOLON)) {
        return AS_STATEMENT(makeReturn(NULL, arena()));
    }

    Expression *expression = parseComma();

    consume(TOKEN_SEMICOLON, "expect ';' after statement");

    return AS_STATEMENT(makeReturn(expression, arena()));
}


static Statement *parseStruct() {

    return NULL;
}


static Statement *parseExpression() {
    Expression *expression = parseComma();
    consume(TOKEN_SEMICOLON, "expect ';' to close a statement");
    return AS_STATEMENT(makeExprStatement(expression, arena()));
}


static Expression *parseComma() {
    Expression *left = parseAssign();

    const Token *token = match(TOKEN_COMMA);
    if (token) {
        Expression *right = parseComma();
        return AS_EXPRESSION(makeComma(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseAssign() {
    Expression *left = parseTernary();

    const Token *token = matchAny((TokenType[]){ TOKEN_EQUAL, TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL, TOKEN_STAR_EQUAL, TOKEN_SLASH_EQUAL, TOKEN_SLASH_EQUAL, TOKEN_STAR_STAR_EQUAL }, 7);

    if (token) {
        Expression *right = parseAssign();
        return AS_EXPRESSION(makeAssign(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseTernary() {
    Expression *condition = parseOr();

    const Token *token = match(TOKEN_QUESTION);
    if (token) {
        Expression *expression1 = parseTernary();
        consume(TOKEN_COLON, "expect ':' after first expression in ternary operator");
        Expression *expression2 = parseTernary();
        return AS_EXPRESSION(makeTernary(*token, condition, expression1, expression2, arena()));
    }
    return condition;
}


static Expression *parseOr() {
    Expression *left = parseAnd();

    const Token *token = NULL;
    for (; (token = match(TOKEN_OR));) {
        Expression *right = parseAnd();
        left = AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseAnd() {
    Expression *left = parseEqual();

    const Token *token = NULL;
    for (; (token = match(TOKEN_AND));) {
        Expression *right = parseEqual();
        left = AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseEqual() {
    Expression *left = parseComparision();

    const Token *token = NULL;
    for (; (token = matchAny((TokenType[]){TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL}, 2));) {
        Expression *right = parseComparision();
        left = AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseComparision() {
    Expression *left = parseTerm();

    const Token *token = NULL;
    for (; (token = matchAny((TokenType[]){ TOKEN_LESSER, TOKEN_LESSER_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL }, 4));) {
        Expression *right = parseTerm();
        left = AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseTerm() {
    Expression *left = parseFactor();

    const Token *token = NULL;
    for (; (token = matchAny((TokenType[]){ TOKEN_PLUS, TOKEN_MINUS }, 2));) {
        Expression *right = parseFactor();
        left = AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseFactor() {
    Expression *left = parsePower();

    const Token *token = NULL;
    for (; (token = matchAny((TokenType[]){ TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT }, 3));) {
        Expression *right = parsePower();
        left = AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parsePower() {
    Expression *left = parseUnary();

    const Token *token = match(TOKEN_STAR_STAR);
    if (token) {
        Expression *right = parsePower();
        return AS_EXPRESSION(makeBinary(*token, left, right, arena()));
    }
    return left;
}


static Expression *parseUnary() {
    const Token *token = matchAny((TokenType[]){ TOKEN_PLUS, TOKEN_MINUS, TOKEN_BANG }, 3);
    if (token) {
        Expression *expression = parseUnary();
        return AS_EXPRESSION(makeUnary(*token, expression, arena()));
    }
    return parseCall();
}


static void appendExpression(Expressions *expressions, Expression *expression) {
    ENSURE_CAPACITY(expressions, Expression*);
    expressions->array[expressions->count] = expression;
    expressions->count++;
}


static Expressions *parseArguments() {
    if (match(TOKEN_RIGHT_PAREN)) {
        return NULL;
    }
    Expressions *expressions = makeExpressions(arena());
    for (;;) {
        appendExpression(expressions, parseAssign());
        if (match(TOKEN_RIGHT_PAREN)) break;
        consume(TOKEN_COMMA, "expect ',' between arguments of function call");
    }
    return expressions;
}


static Expression *parseCall() {
    Expression *callee = parseTerminal();

    const Token *token = match(TOKEN_LEFT_PAREN);
    for (;token;) {
        return AS_EXPRESSION(makeCall(*token, callee, parseArguments(), arena()));
    }

    return callee;
}


static Expression *parseTerminal() {
    const Token *token = NULL;
    if ((token = matchAny((TokenType[]){ TOKEN_NUMBER, TOKEN_STRING, TOKEN_NIL, TOKEN_TRUE, TOKEN_FALSE, TOKEN_IDENTIFIER }, 6))) {
        return AS_EXPRESSION(makeTerminal(*token, arena()));
    }

    if ((token = match(TOKEN_LEFT_PAREN))) {
        Expression *expression = parseComma();
        consume(TOKEN_RIGHT_PAREN, "expect ')' to close '('");
        return AS_EXPRESSION(makeGroup(*token, expression, arena()));
    }

    if ((token = match(TOKEN_FN))) {
        return AS_EXPRESSION(parseFnExpression());
    }

    parseError("expect a number, string, nil boolean, group or function expresion");
    return NULL;
}


static Statement *parseStatement() {
    switch (forward()->type) {
        case TOKEN_PUT: return parsePut();
        case TOKEN_PUTLN: return parsePutln();
        case TOKEN_VAR: return parseVar();
        case TOKEN_CONST: return parseConst();
        case TOKEN_LEFT_BRACE: return parseBlock();
        case TOKEN_IF: return parseIf();
        case TOKEN_FOR: return parseFor();
        case TOKEN_BREAK: return parseBreak();
        case TOKEN_CONTINUE: return parseContinue();
        case TOKEN_FN: return parseFn();
        case TOKEN_RETURN: return parseReturn();
        case TOKEN_STRUCT: return parseStruct();
        default: {
            backward();
            return parseExpression();
        }
    }
}


static void skipToNext() {
    for (;;) {
        switch (current()->type) {
            case TOKEN_SEMICOLON:
                forward();
            case TOKEN_PUT:
            case TOKEN_PUTLN:
            case TOKEN_VAR:
            case TOKEN_CONST:
            case TOKEN_LEFT_BRACE:
            case TOKEN_IF:
            case TOKEN_FOR:
            case TOKEN_FN:
            case TOKEN_STRUCT:
            case TOKEN_EOF:
                return;
            default:
                forward();
        }
    }
}


static void initStatements(Statements *statements) {
    statements->array = NULL;
    statements->capacity = 0;
    statements->count = 0;
}


static void appendStatement(Statements *statements, Statement *statement) {
    ENSURE_CAPACITY(statements, Statement*);
    statements->array[statements->count] = statement;
    statements->count++;
}


bool parseStatements(Statements *statements, const Tokens *tokens, Arena *arena) {
    initStatements(statements);
    initParser(statements, tokens->array, arena);

    if (setjmp(buf) == 1) {
        skipToNext();
    }

    for (;!isAtEnd();) {
        appendStatement(statements, parseStatement());
    }

#ifdef DEBUG_PRINT_AST
    PRINT_AST(statements);
#endif

    bool result = !hadErrors();
    freeParser();
    return result;
}


void freeStatements(Statements *statements) {
    statements->capacity = 0;
    statements->count = 0;
    statements->array = FREE(statements->array, Statement*);
}
