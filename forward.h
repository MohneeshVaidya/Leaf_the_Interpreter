#ifndef leaf_forward_h
#define leaf_forward_h


#include <stdint.h>


#include "expression.h"


typedef struct Interpreter Interpreter;
typedef struct Environment Environment;


typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjFn ObjFn;
typedef struct ObjStruct ObjStruct;
typedef struct ObjStructValue ObjStructValue;

typedef struct Value Value;

typedef struct Entry Entry;
typedef struct Table Table;


typedef enum ObjType {
    OBJ_STRING,
    OBJ_ENVIRONMENT,
    OBJ_FN,
    OBJ_STRUCT,
    OBJ_STRUCT_VALUE
} ObjType;


struct Obj {
    ObjType type;
    struct Obj *next;
    bool isMarked;
};


struct ObjString {
    Obj meta;
    char *chars;
    int length;
    uint32_t hash;
};


struct ObjFn {
    Obj meta;
    Parameter parameters[MAX_PARAMETERS];
    int arity;
    struct Block *block;
    struct Environment *closure;
};


struct ObjStruct {
    Obj meta;
    struct Block *block;
    struct Environment *closure;
};


struct ObjStructValue {
    Obj meta;
    struct Environment *context;
};


typedef enum ValueType {
    VALUE_NIL,
    VALUE_TOMBSTONE,
    VALUE_NUMBER,
    VALUE_BOOLEAN,
    VALUE_OBJ,
} ValueType;


struct Value {
    ValueType type;
    union {
        double number;
        bool boolean;
        Obj *obj;
    } as;
};


struct Entry {
    ObjString *key;
    Value value;
};


struct Table {
    int capacity;
    int count;
    Entry *array;
};


struct Environment {
    Obj meta;
    Environment *previous;
    Environment *saved;
    Table table;
};


struct Interpreter {
    Environment *env;
    Table strings;
    bool isLoop;
    bool isFunction;
    int loopIdx;
    int functionIdx;
    Value returnValue;
    Obj *objects;
};


#endif
