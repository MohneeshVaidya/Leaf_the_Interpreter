#ifndef leaf_value_h
#define leaf_value_h


#include <stdbool.h>


#include "object.h"
#include "tokenizer.h"


#define NIL_VALUE               ((Value){ VALUE_NIL, { .number = 0 } })
#define NUMBER_VALUE(value)     ((Value){ VALUE_NUMBER, { .number = value } })
#define BOOLEAN_VALUE(value)    ((Value){ VALUE_BOOLEAN, { .boolean = value } })
#define OBJ_VALUE(value)        ((Value){ VALUE_OBJ, { .obj = (Obj *)value } })


#define IS_NIL(value)           ((value).type == VALUE_NIL)
#define IS_NUMBER(value)        ((value).type == VALUE_NUMBER)
#define IS_BOOLEAN(value)       ((value).type == VALUE_BOOLEAN)
#define IS_OBJ(value)           ((value).type == VALUE_OBJ)
#define IS_STRING(value)        (isObjType(value, OBJ_STRING))


#define AS_NUMBER(value)        ((value).as.number)
#define AS_BOOLEAN(value)       ((value).as.boolean)
#define AS_OBJ(value)           ((value).as.obj)
#define AS_STRING(value)        ((ObjString *)AS_OBJ(value))


typedef enum ValueType {
    VALUE_NIL,
    VALUE_NUMBER,
    VALUE_BOOLEAN,
    VALUE_OBJ,
} ValueType;


typedef struct Value {
    ValueType type;
    union {
        double number;
        bool boolean;
        Obj *obj;
    } as;
} Value;


void printValue(Value value);
bool isTruthy(Value a);
Value performBinary(Value a, Value b, Token token);
Value performUnary(Value a, Token token);


static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && (AS_OBJ(value)->type == type);
}


#endif
