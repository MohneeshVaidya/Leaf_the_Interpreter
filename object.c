#include <stdio.h>
#include <string.h>


#include "object.h"
#include "expression.h"
#include "memory.h"
#include "table.h"
#include "value.h"


// #define DEBUG_STRING_INTERNING


#define INITIAL_HASH (2166136u)


#define MAKE_OBJ(typeTag, interpreter, type) \
    ((type *)(makeObj(typeTag, sizeof(type), interpreter)))


static void printString(ObjString *obj) {
    printf("%s", obj->chars);
}


static void printObjStructValue(ObjStructValue *obj) {
#define GET_TABLE(obj)  ((obj)->context->table)

    printf("{\n");
    for (int i = 0; i < GET_TABLE(obj).capacity; i++) {
        Entry *entry = GET_TABLE(obj).array + i;
        if (entry->key != NULL) {
            printf("\t");
            printString(entry->key);
            printf(" : ");
            printValue(entry->value);
            printf("\n");
        }
    }
    printf("}");

#undef GET_TABLE
}


void printObj(const Obj *obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            return printString((ObjString *)(obj));
        }
        case OBJ_FN: {
            printf("fn () { }");
            return;
        }
        case OBJ_STRUCT: {
            printf("struct { }");
            return;
        }
        case OBJ_STRUCT_VALUE: {
            return printObjStructValue((ObjStructValue *)obj);
        }
        case OBJ_ENVIRONMENT: {
            printf("Environment");
            return;
        }
        default:
            printf("obj");
            break;
    }
}


static uint32_t hash(const char *chars, int length, uint32_t initialHash) {
    uint32_t hash_ = initialHash;
    for (int i = 0; i < length; i++) {
        hash_ ^= chars[i];
        hash_ *= 16777619u;
    }
    return hash_;
}


static ObjString *searchKey(const char *chars, int length, uint32_t hash_, const Table *strings) {
    int index = hash_ % strings->capacity;
    for (int i = 0; i < strings->capacity; i++) {
        const Entry *entry = strings->array + index;

        if (entry->key == NULL) {
            return NULL;
        }
        if ((entry->key->length == length) && (memcmp(entry->key->chars, chars, length) == 0)) {
            return entry->key;
        }
        index = (index + 1) % strings->capacity;
    }
    return NULL;
}


static ObjString *searchKeyTwoStrings(const char *chars1, int length1, const char *chars2, int length2, uint32_t hash_, const Table *strings) {
    int index = hash_ % strings->capacity;
    for (int i = 0; i < strings->capacity; i++) {
        const Entry *entry = strings->array + index;

        if (entry->key == NULL) {
            return NULL;
        }
        if (
            (entry->key->length == length1 + length2) &&
            (memcmp(entry->key->chars, chars1, length1) == 0) &&
            (memcmp(entry->key->chars + length1, chars2, length2) == 0)) {
            return entry->key;
        }
        index = (index + 1) % strings->capacity;
    }
    return NULL;
}


static Obj *makeObj(ObjType typeTag, int bytes, Interpreter *interpreter) {
    Obj *obj = REALLOCATE(NULL, 0, bytes, interpreter, Obj);

    obj->type = typeTag;
    obj->next = interpreter->objects;
    interpreter->objects = obj;
    obj->isMarked = false;

    return obj;
}


ObjString *internString(const char *chars, int length, Interpreter *interpreter) {
    uint32_t hash_ = hash(chars, length, INITIAL_HASH);

    ObjString *string = searchKey(chars, length, hash_, &interpreter->strings);
    if (string) {
        return string;
    }

#ifdef DEBUG_STRING_INTERNING
    printf("Creating string '%.*s'\n", length, chars);
#endif

    string = MAKE_OBJ(OBJ_STRING, interpreter, ObjString);

    string->chars = REALLOCATE(NULL, 0, length + 1, interpreter, char);

    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    string->length = length;
    string->hash = hash_;

    tableSet(&interpreter->strings, string, NIL_VALUE, interpreter);

    return string;
}


ObjString *addStrings(const ObjString *a, const ObjString *b, Interpreter *interpreter) {

    uint32_t hash_ = hash(b->chars, b->length, hash(a->chars, a->length, INITIAL_HASH));

    ObjString *string = searchKeyTwoStrings(a->chars, a->length, b->chars, b->length, hash_, &interpreter->strings);
    if (string) {
        return string;
    }

#ifdef DEBUG_STRING_INTERNING
    printf("Adding strings '%.*s' & '%.*s'\n", a->length, a->chars, b->length, b->chars);
#endif

    string = MAKE_OBJ(OBJ_STRING, interpreter, ObjString);

    int length = a->length + b->length;

    string->chars = REALLOCATE(NULL, 0, length + 1, interpreter, char);
    memcpy(string->chars, a->chars, a->length);
    memcpy(string->chars + a->length, b->chars, b->length);
    string->chars[length] = '\0';

    string->length = length;
    string->hash = hash_;

    tableSet(&interpreter->strings, string, NIL_VALUE, interpreter);

    return string;
}


static int compareStrings(const ObjString *a, const ObjString *b) {
    return strcmp(a->chars, b->chars);
}


bool equalStrings(const ObjString *a, const ObjString *b) {
    if (a && b) {
        return (a == b);
    }
    return false;
}


ObjFn *makeObjFn(Parameter *parameters, int arity, struct Block *block, struct Environment *closure, Interpreter *interpreter) {
    ObjFn *obj = MAKE_OBJ(OBJ_FN, interpreter, ObjFn);

    for (int i = 0; i < arity; i++) {
        obj->parameters[i] = parameters[i];
    }

    obj->arity = arity;
    obj->block = block;
    obj->closure = closure;

    return obj;
}


ObjStruct *makeObjStruct(struct Block *block, struct Environment *closure, Interpreter *interpreter) {
    ObjStruct *obj = MAKE_OBJ(OBJ_STRUCT, interpreter, ObjStruct);
    obj->block = block;
    obj->closure = closure;
    return obj;
}


ObjStructValue *makeObjStructValue(struct Environment *context, Interpreter *interpreter) {
    ObjStructValue *obj = MAKE_OBJ(OBJ_STRUCT_VALUE, interpreter, ObjStructValue);
    obj->context = context;
    return obj;
}
