#ifndef leaf_object_h
#define leaf_object_h


#include <stdint.h>
#include <stdbool.h>


typedef enum ObjType {
    OBJ_STRING,
    OBJ_FN,
    OBJ_STRUCT,
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
} ObjFn;


typedef struct ObjStruct {
    Obj meta;
} ObjStruct;


ObjString *makeString(const char *chars, int length);
ObjString *addStrings(const ObjString *a, const ObjString *b);
bool equalStrings(const ObjString *a, const ObjString *b);
void printObj(const Obj *obj);


#endif
