#ifndef leaf_object_h
#define leaf_object_h


#include <stdint.h>
#include <stdbool.h>


#include "expression.h"


typedef enum ObjType {
    OBJ_STRING,
    OBJ_FN,
    OBJ_STRUCT,
    OBJ_STRUCT_VALUE
} ObjType;


typedef struct Obj {
    ObjType type;
} Obj;


typedef struct ObjString {
    Obj meta;
    char *chars;
    int length;
    uint32_t hash;
} ObjString;


typedef struct ObjFn {
    Obj meta;
    Parameter parameters[MAX_PARAMETERS];
    int arity;
    struct Block *block;
    struct Environment *closure;
} ObjFn;


struct Table;

typedef struct ObjStruct {
    Obj meta;
    struct Block *block;
    struct Environment *closure;
} ObjStruct;


typedef struct ObjStructValue {
    Obj meta;
    struct Environment *context;
} ObjStructValue;


void printObj(const Obj *obj);
ObjString *internString(const char *chars, int length, struct Table *strings);
ObjString *addStrings(const ObjString *a, const ObjString *b, struct Table *strings);
bool equalStrings(const ObjString *a, const ObjString *b);
ObjFn *makeObjFn(Parameter *parameters, int arity, struct Block *block, struct Environment *closure);
ObjStruct *makeObjStruct(struct Block *block, struct Environment *closure);
ObjStructValue *makeObjStructValue(struct Environment *context);


#endif
