#include <stdio.h>
#include <math.h>
#include <string.h>


#include "object.h"
#include "value.h"
#include "interpreter.h"
#include "tokenizer.h"


#define NUMBER_PERFORM_BINARY(a, b, op) (NUMBER_VALUE(AS_NUMBER(a) op AS_NUMBER(b)))


void printValue(Value value) {
    switch (value.type) {
        case VALUE_NIL: {
            printf("nil");
            break;
        }
        case VALUE_NUMBER: {
            printf("%.2lf", AS_NUMBER(value));
            break;
        }
        case VALUE_BOOLEAN: {
            printf("%s", AS_BOOLEAN(value) ? "true" : "false");
            break;
        }
        case VALUE_OBJ: {
            printObj(AS_OBJ(value));
            break;
        }
        default:
            break;
    }
}


 bool isTruthy(Value a) {
    if (IS_NUMBER(a)) {
        return !!AS_NUMBER(a);
    }
    if (IS_BOOLEAN(a)) {
        return AS_BOOLEAN(a);
    }
    if (IS_NIL(a)) {
        return false;
    }
    if (IS_STRING(a)) {
        char *chars = AS_STRING(a)->chars;
        return strcpy(chars, "") > 0;
    }
    return false;
}


static Value add(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return NUMBER_PERFORM_BINARY(a, b, +);
    }
    if (IS_STRING(a) && IS_STRING(b)) {
        return OBJ_VALUE(addStrings(AS_STRING(a), AS_STRING(b)));
    }
    runtimeError("'+' expects strings or numbers as operands", token.line);
    return NIL_VALUE;
}


static Value subtract(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return NUMBER_PERFORM_BINARY(a, b, -);
    }
    runtimeError("'-' expects only numbers as operands", token.line);
    return NIL_VALUE;
}


static Value multiply(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return NUMBER_PERFORM_BINARY(a, b, *);
    }
    runtimeError("'*' expects only numbers as operands", token.line);
    return NIL_VALUE;
}


static Value divide(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        if (AS_NUMBER(b) == 0) {
            runtimeError("division by 0 error", token.line);
            return NIL_VALUE;
        }
        return NUMBER_PERFORM_BINARY(a, b, /);
    }
    runtimeError("'/' expects only numbers as operands", token.line);
    return NIL_VALUE;
}


static Value mod(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return NUMBER_VALUE(fmod(AS_NUMBER(a), AS_NUMBER(b)));
    }
    runtimeError("'%' expects only numbers as operands", token.line);
    return NIL_VALUE;
}


static Value power(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return NUMBER_VALUE(pow(AS_NUMBER(a), AS_NUMBER(b)));
    }
    runtimeError("'**' expects only numbers as operands", token.line);
    return NIL_VALUE;
}


static Value and_(Value a, Value b) {
    return BOOLEAN_VALUE(isTruthy(a) && isTruthy(b));
}


static Value or_(Value a, Value b) {
    return BOOLEAN_VALUE(isTruthy(a) || isTruthy(b));
}


static Value equal(Value a, Value b, Token token) {
    if (a.type != b.type) {
        return BOOLEAN_VALUE(false);
    }
    if (IS_NUMBER(a)) {
        return BOOLEAN_VALUE(AS_NUMBER(a) == AS_NUMBER(b));
    }
    if (IS_BOOLEAN(a)) {
        return BOOLEAN_VALUE(AS_BOOLEAN(a) == AS_BOOLEAN(b));
    }
    if (IS_NIL(a)) {
        return BOOLEAN_VALUE(true);
    }
    return BOOLEAN_VALUE(false);
}


static Value notEqual(Value a, Value b, Token token) {
    return BOOLEAN_VALUE(!AS_BOOLEAN(equal(a, b, token)));
}


static Value lesser(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return BOOLEAN_VALUE(AS_NUMBER(a) < AS_NUMBER(b));
    }
    runtimeError("'<' expects both operands to be either number or string", token.line);
    return BOOLEAN_VALUE(false);
}


static Value lesserEqual(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return BOOLEAN_VALUE(AS_NUMBER(a) <= AS_NUMBER(b));
    }
    runtimeError("'<=' expects both operands to be either number or string", token.line);
    return BOOLEAN_VALUE(false);
}


static Value greater(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return BOOLEAN_VALUE(AS_NUMBER(a) > AS_NUMBER(b));
    }
    runtimeError("'>' expects both operands to be either number or string", token.line);
    return BOOLEAN_VALUE(false);
}


static Value greaterEqual(Value a, Value b, Token token) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return BOOLEAN_VALUE(AS_NUMBER(a) >= AS_NUMBER(b));
    }
    runtimeError("'>=' expects both operands to be either number or string", token.line);
    return BOOLEAN_VALUE(false);
}


Value performBinary(Value a, Value b, Token token) {
    switch (token.type) {
        case TOKEN_PLUS: {
            return add(a, b, token);
        }
        case TOKEN_MINUS: {
            return subtract(a, b, token);
        }
        case TOKEN_STAR: {
            return multiply(a, b, token);
        }
        case TOKEN_SLASH: {
            return divide(a, b, token);
        }
        case TOKEN_PERCENT: {
            return mod(a, b, token);
        }
        case TOKEN_STAR_STAR: {
            return power(a, b, token);
        }
        case TOKEN_AND: {
            return and_(a, b);
        }
        case TOKEN_OR: {
            return or_(a, b);
        }
        case TOKEN_EQUAL_EQUAL: {
            return equal(a, b, token);
        }
        case TOKEN_BANG_EQUAL: {
            return notEqual(a, b, token);
        }
        case TOKEN_LESSER: {
            return lesser(a, b, token);
        }
        case TOKEN_LESSER_EQUAL: {
            return lesserEqual(a, b, token);
        }
        case TOKEN_GREATER: {
            return greater(a, b, token);
        }
        case TOKEN_GREATER_EQUAL: {
            return greaterEqual(a, b, token);
        }
        default:
            break;
    }
    return NIL_VALUE;
}


Value performUnary(Value a, Token token) {
    switch (token.type) {
        case TOKEN_PLUS: {
            if (IS_NUMBER(a)) {
                return a;
            }
            runtimeError("unary '+' expects only number as argument", token.line);
            break;
        }
        case TOKEN_MINUS: {
            if (IS_NUMBER(a)) {
                return NUMBER_VALUE(-AS_NUMBER(a));
            }
            runtimeError("unary '-' expects only number as argument", token.line);
            break;
        }
        case TOKEN_BANG: {
            return BOOLEAN_VALUE(!isTruthy(a));
        }
        default:
            break;
    }
    return NIL_VALUE;
}
