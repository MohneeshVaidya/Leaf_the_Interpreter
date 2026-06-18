#include <stdio.h>
#include <string.h>


#include "object.h"
#include "memory.h"


#define MAKE_OBJ(typeTag, type) \
    ((type *)(makeObj(typeTag, sizeof(type))))


static uint32_t hash(const char *chars, int length) {
    uint32_t hash_ = 2166136u;
    for (int i = 0; i < length; i++) {
        hash_ ^= chars[i];
        hash_ *= 16777619u;
    }
    return hash_;
}


static Obj *makeObj(ObjType typeTag, int bytes) {
    Obj *obj = REALLOCATE(NULL, bytes, Obj);
    obj->type = typeTag;
    return obj;
}


ObjString *makeString(const char *chars, int length) {
    ObjString *obj = MAKE_OBJ(OBJ_STRING, ObjString);

    obj->chars = NULL;
    obj->chars = REALLOCATE(obj->chars, length + 1, char);

    memcpy(obj->chars, chars, length);

    obj->chars[length] = '\0';
    obj->length = length;
    obj->hash = hash(obj->chars, obj->length);

    return obj;
}


ObjString *addStrings(const ObjString *a, const ObjString *b) {
    ObjString *obj = MAKE_OBJ(OBJ_STRING, ObjString);

    obj->length = a->length + b->length;

    obj->chars = NULL;
    obj->chars = REALLOCATE(obj->chars, obj->length + 1, char);

    memcpy(obj->chars, a->chars, a->length);
    memcpy(obj->chars + a->length, b->chars, b->length);

    obj->chars[obj->length] = '\0';
    obj->hash = hash(obj->chars, obj->length);

    return obj;
}


static int compareStrings(const ObjString *a, const ObjString *b) {
    return strcmp(a->chars, b->chars);
}


bool equalStrings(const ObjString *a, const ObjString *b) {
    if (a && b) {
        return (a->hash == b->hash) && (compareStrings(a, b) == 0);
    }
    return false;
}


static void printString(ObjString *obj) {
    printf("%s", obj->chars);
}


void printObj(const Obj *obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            return printString((ObjString *)obj);
        }
        default:
            printf("obj");
            break;
    }
}
