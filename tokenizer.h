#ifndef leaf_tokenizer_h
#define leaf_tokenizer_h


#include <stdbool.h>


typedef enum TokenType {
    TOKEN_PUT,
    TOKEN_PUTLN,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSEIF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_FN,
    TOKEN_RETURN,
    TOKEN_STRUCT,
    TOKEN_MAKE,
    TOKEN_THIS,
    TOKEN_EXTENDS,
    TOKEN_SUPER,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_XOR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_FORMAT_STRING,
    TOKEN_NUMBER,
    TOKEN_NIL,
    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_PLUS,
    TOKEN_PLUS_EQUAL,
    TOKEN_MINUS,
    TOKEN_MINUS_EQUAL,
    TOKEN_STAR,
    TOKEN_STAR_STAR,
    TOKEN_STAR_EQUAL,
    TOKEN_STAR_STAR_EQUAL,
    TOKEN_SLASH,
    TOKEN_SLASH_EQUAL,
    TOKEN_PERCENT,
    TOKEN_PERCENT_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_LESSER,
    TOKEN_LESSER_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_EQUAL_GREATER,

    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_DOT,
    TOKEN_QUESTION,
} TokenType;


typedef struct Token {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;


typedef struct Tokens {
    int capacity;
    int count;
    Token *array;
} Tokens;


bool makeTokens(Tokens *tokens, const char *source);
void freeTokens(Tokens *tokens);


#endif
