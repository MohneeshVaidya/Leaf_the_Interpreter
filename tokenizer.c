#include <stdio.h>
#include <string.h>
#include <stddef.h>


#include "tokenizer.h"
#include "memory.h"


// #define DEBUG_PRINT_TOKENS


#ifdef DEBUG_PRINT_TOKENS
#include "token_printer.h"
#endif


typedef struct Tokenizer {
    const char *source;
    size_t sourceLength;
    const char *current;
    Tokens *tokens;
    int line;
    bool hadErrors;
} Tokenizer;


static Tokenizer tokenizer;


static void initTokenizer(const char *source, Tokens *tokens) {
    tokenizer.source = source;
    tokenizer.sourceLength = strlen(source);
    tokenizer.current = tokenizer.source;
    tokenizer.tokens = tokens;
    tokenizer.line = 1;
    tokenizer.hadErrors = false;
}


static void freeTokenizer() {
    tokenizer.source = NULL;
    tokenizer.sourceLength = 0;
    tokenizer.tokens = NULL;
    tokenizer.current = NULL;
    tokenizer.line = 0;
    tokenizer.hadErrors = false;
}


static inline const char *source() { return tokenizer.source; }
static inline size_t sourceLength() { return tokenizer.sourceLength; }
static inline const char *current() { return tokenizer.current; }
static inline Tokens *tokens() { return tokenizer.tokens; }
static inline int line() { return tokenizer.line; }
static inline bool hadErrors() { return tokenizer.hadErrors; };


static char peekAt(int index) {
    if (((current() - source()) + index) >= sourceLength()) {
        return '\0';
    }

    return *(tokenizer.current + index);
}


static inline char peek() {
    return peekAt(0);
}


static inline void forward() {
    if (peek() != '\0') tokenizer.current++;
}


static bool isWs(char ch) {
    switch (ch) {
        case ' ':
        case '\t':
        case '\b':
        case '\r':
        case '\n':
            return true;
    }
    return false;
}


static inline bool isDigit(char ch) {
    return ('0' <= ch) && (ch <= '9');
}


static inline bool isAlpha(char ch) {
    return ('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z') || (ch == '_');
}


static inline bool isAlphaNum(char ch) {
    return isAlpha(ch) || isDigit(ch);
}


static void skipWs() {
    for(; isWs(peek()) ;) {
        if (peek() == '\n') tokenizer.line++;
        forward();
    }
}


static void skipComment() {
    if (peek() != '/' || peekAt(1) != '*') return;

    forward();
    forward();

    for (;;) {
        if (peek() == '*' && peekAt(1) == '/') {
            forward();
            forward();
            break;
        }
        if (peek() == '\n') tokenizer.line++;
        if (peek() == '\0') return;
        forward();
    }
}


static inline Token makeToken(TokenType type, const char *start, int length, int line) {
    return (Token){ type, start, length, line };
}


// static inline void ensureCapacity() {
//     if (tokens()->count >= tokens()->capacity) {
//         int newCapacity = GROW_CAPACITY(tokens()->capacity);
//         tokens()->array = REALLOCATE(tokens()->array, newCapacity, Token);
//         tokens()->capacity = newCapacity;
//     }
// }


static void appendToken(Tokens *tokens, TokenType type, const char *start, int length, int line) {
    ENSURE_CAPACITY(tokens, Token);
    tokens->array[tokens->count] = makeToken(type, start, length, line);
    tokens->count++;

    tokenizer.current +=  length;
}


static void appendErrorToken(Tokens *tokens, const char *message, int line) {
    tokenizer.hadErrors = true;
    appendToken(tokens, TOKEN_ERROR, message, strlen(message), line);
}


static void string() {
    forward();

    int line_ = line();
    int length = 0;
    for (; peekAt(length) != '\0' && peekAt(length) != '"'; length++) {
        if (peekAt(length) == '\n') tokenizer.line++;
    }
    if (peekAt(length) == '\0') {
        appendErrorToken(tokens(), "trailing '\"' is missing for the string", line_);
        return;
    }
    appendToken(tokens(), TOKEN_STRING, current(), length, line_);
    forward();
}


static void number() {
    int length = 1;
    int dotSeen = 0;
    for (; isDigit(peekAt(length)) || peekAt(length) == '.'; length++) {
        if (peekAt(length) == '.') {
            dotSeen++;
            if (dotSeen > 1) break;
        }
    }
    appendToken(tokens(), TOKEN_NUMBER, current(), length, line());
}


static inline bool isSame(const char *start, size_t length, const char *other) {
    return (length == strlen(other)) && (strncmp(start, other, length) == 0);
}


static bool keyword(const char *start, size_t length) {
    if (isSame(start, length, "put")) {
        appendToken(tokens(), TOKEN_PUT, start, length, line());
        return true;
    } else if (isSame(start, length, "putln")) {
        appendToken(tokens(), TOKEN_PUTLN, start, length, line());
        return true;
    } else if (isSame(start, length, "var")) {
        appendToken(tokens(), TOKEN_VAR, start, length, line());
        return true;
    } else if (isSame(start, length, "const")) {
        appendToken(tokens(), TOKEN_CONST, start, length, line());
        return true;
    } else if (isSame(start, length, "if")) {
        appendToken(tokens(), TOKEN_IF, start, length, line());
        return true;
    } else if (isSame(start, length, "elseif")) {
        appendToken(tokens(), TOKEN_ELSEIF, start, length, line());
        return true;
    } else if (isSame(start, length, "else")) {
        appendToken(tokens(), TOKEN_ELSE, start, length, line());
        return true;
    } else if (isSame(start, length, "for")) {
        appendToken(tokens(), TOKEN_FOR, start, length, line());
        return true;
    } else if (isSame(start, length, "continue")) {
        appendToken(tokens(), TOKEN_CONTINUE, start, length, line());
        return true;
    } else if (isSame(start, length, "break")) {
        appendToken(tokens(), TOKEN_BREAK, start, length, line());
        return true;
    } else if (isSame(start, length, "fn")) {
        appendToken(tokens(), TOKEN_FN, start, length, line());
        return true;
    } else if (isSame(start, length, "return")) {
        appendToken(tokens(), TOKEN_RETURN, start, length, line());
        return true;
    } else if (isSame(start, length, "struct")) {
        appendToken(tokens(), TOKEN_STRUCT, start, length, line());
        return true;
    } else if (isSame(start, length, "make")) {
        appendToken(tokens(), TOKEN_MAKE, start, length, line());
        return true;
    } else if (isSame(start, length, "this")) {
        appendToken(tokens(), TOKEN_THIS, start, length, line());
        return true;
    } else if (isSame(start, length, "extends")) {
        appendToken(tokens(), TOKEN_EXTENDS, start, length, line());
        return true;
    } else if (isSame(start, length, "super")) {
        appendToken(tokens(), TOKEN_SUPER, start, length, line());
        return true;
    } else if (isSame(start, length, "and")) {
        appendToken(tokens(), TOKEN_AND, start, length, line());
        return true;
    } else if (isSame(start, length, "or")) {
        appendToken(tokens(), TOKEN_OR, start, length, line());
        return true;
    } else if (isSame(start, length, "xor")) {
        appendToken(tokens(), TOKEN_XOR, start, length, line());
        return true;
    }
    return false;
}


static void identifier() {
    int length = 1;
    for (; isAlphaNum(peekAt(length)); length++);

    if (keyword(current(), length)) return;

    appendToken(tokens(), TOKEN_IDENTIFIER, current(), length, line());
}


static void scanTokens() {
    skipWs();
    skipComment();

    char ch = peek();

    if (ch == '"') return string();
    if (isDigit(ch)) return number();
    if (isAlpha(ch)) return identifier();

    switch (ch) {
        case '+': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_PLUS_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_PLUS, current(), 1, line());
            }
            break;
        }
        case '-': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_MINUS_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_MINUS, current(), 1, line());
            }
            break;
        }
        case '*': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_STAR_EQUAL, current(), 2, line());
            } else if (peekAt(1) == '*') {
                if (peekAt(2) == '=') {
                    appendToken(tokens(), TOKEN_STAR_STAR_EQUAL, current(), 3, line());
                } else {
                    appendToken(tokens(), TOKEN_STAR_STAR, current(), 2, line());
                }
            } else {
                appendToken(tokens(), TOKEN_STAR, current(), 1, line());
            }
            break;
        }
        case '/':
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_SLASH_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_SLASH, current(), 1, line());
            }
            break;
        case '%': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_PERCENT_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_PERCENT, current(), 1, line());
            }
            break;
        }
        case '=': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_EQUAL_EQUAL, current(), 2, line());
            } else if (peekAt(1) == '>') {
                appendToken(tokens(), TOKEN_EQUAL_GREATER, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_EQUAL, current(), 1, line());
            }
            break;
        }
        case '!': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_BANG_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_BANG, current(), 1, line());
            }
            break;
        }
        case '<': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_LESSER_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_LESSER, current(), 1, line());
            }
            break;
        }
        case '>': {
            if (peekAt(1) == '=') {
                appendToken(tokens(), TOKEN_GREATER_EQUAL, current(), 2, line());
            } else {
                appendToken(tokens(), TOKEN_GREATER, current(), 1, line());
            }
            break;
        }
        case ';':
            appendToken(tokens(), TOKEN_SEMICOLON, current(), 1, line());
            break;
        case ':':
            appendToken(tokens(), TOKEN_COLON, current(), 1, line());
            break;
        case ',':
            appendToken(tokens(), TOKEN_COMMA, current(), 1, line());
            break;
        case '(':
            appendToken(tokens(), TOKEN_LEFT_PAREN, current(), 1, line());
            break;
        case ')':
            appendToken(tokens(), TOKEN_RIGHT_PAREN, current(), 1, line());
            break;
        case '{':
            appendToken(tokens(), TOKEN_LEFT_BRACE, current(), 1, line());
            break;
        case '}':
            appendToken(tokens(), TOKEN_RIGHT_BRACE, current(), 1, line());
            break;
        case '.':
            appendToken(tokens(), TOKEN_DOT, current(), 1, line());
            break;
        case '?':
            appendToken(tokens(), TOKEN_QUESTION, current(), 1, line());
            break;
        default:
            forward();
    }
}


bool makeTokens(Tokens *tokens_, const char *source) {
    tokens_->array = NULL;
    tokens_->capacity = 0;
    tokens_->count = 0;


    initTokenizer(source, tokens_);

    for (;!hadErrors() && peek() != '\0';) {
        scanTokens();
    }

    appendToken(tokens_, TOKEN_EOF, "", 0, line());

#ifdef DEBUG_PRINT_TOKENS
    printTokens(tokens());
#endif

    bool result = !hadErrors();
    freeTokenizer();
    return result;
}


void freeTokens(Tokens *tokens) {
    tokens->capacity = 0;
    tokens->count = 0;
    tokens->array = FREE(tokens->array, Token);
}
