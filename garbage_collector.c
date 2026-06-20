#include "garbage_collector.h"
#include "environment.h"
#include "forward.h"
#include "memory.h"
#include "parser.h"
#include "value.h"


#define MARK_OBJECT(object) ((object)->meta.isMarked = true)


static void markObject(Obj *object);
static void markTable(Table *table);


static void freeString(ObjString *string) {
    int length = string->length + 1;
    string->length = 0;
    string->hash = 0;
    string->chars = FREE(string->chars, length, char);
    FREE(string, 1, ObjString);
}


static void freeEnvironment(Environment *env) {
    freeEnv(env);
    FREE(env, 1, Environment);
}


static void freeFn(ObjFn *function) {
    freeStatements(function->block->statements);
    FREE(function, 1, ObjFn);
}


static void freeStruct(ObjStruct *structure) {
    freeStatements(structure->block->statements);
    FREE(structure, 1, ObjStruct);
}


static void freeStructValue(ObjStructValue *structure) {
    FREE(structure, 1, ObjStructValue);
}


static void freeObject(Obj *object) {
    switch (object->type) {
        case OBJ_STRING: {
            return freeString((ObjString *)object);
        }
        case OBJ_ENVIRONMENT: {
            return freeEnvironment((Environment *)object);
        }
        case OBJ_FN: {
            return freeFn((ObjFn *)object);
        }
        case OBJ_STRUCT: {
            return freeStruct((ObjStruct *)object);
        }
        case OBJ_STRUCT_VALUE: {
            return freeStructValue((ObjStructValue *)object);
        }
    }
}


static Obj *sweepObject(Obj **objects, Obj *previous, Obj *current) {
    if (current->isMarked) {
        current->isMarked = false;
        return current;
    }

    Obj *object = current;
    if (!previous) {
        *objects = current->next;
        freeObject(object);
        return NULL;
    }

    previous->next = current->next;

    freeObject(object);
    return previous;
}


static void sweep(Obj **objects) {
    Obj *previous = NULL;
    Obj *current = *objects;
    for (;current;) {
        Obj *next = current->next;

        previous = sweepObject(objects, previous, current);

        current = next;
    }
}


static void markEnvironment(Environment *env) {
    if (!env) {
        return;
    }
    markTable(&env->table);
    markObject((Obj *)env->previous);
    markObject((Obj *)env->saved);
}


static void markFn(ObjFn *function) {
    markEnvironment(function->closure);
}


static void markStruct(ObjStruct *structure) {
    markEnvironment(structure->closure);
}


static void markStructValue(ObjStructValue *structure) {
    markEnvironment(structure->context);
}


static void markObject(Obj *object) {
    if (!object || object->isMarked == true) {
        return;
    }

    object->isMarked = true;

    switch (object->type) {
        case OBJ_ENVIRONMENT: {
            return markEnvironment((Environment *)object);
        }
        case OBJ_FN: {
            return markFn((ObjFn *)object);
        }
        case OBJ_STRUCT: {
            return markStruct((ObjStruct *)object);
        }
        case OBJ_STRUCT_VALUE: {
            return markStructValue((ObjStructValue *)object);
        }
        default: {
            return;
        }
    }
}


static void markTable(Table *table) {
    if (!table) {
        return;
    }
    int index = 0;
    for (;
        index < table->capacity;
        index++
    ) {
        Entry *entry = table->array + index;
        if (entry->key) {
            entry->key->meta.isMarked = true;
            if (IS_OBJ(entry->value)) {
                markObject(AS_OBJ(entry->value));
            }
        }
    }
}


static void mark(Environment *env) {
    if (!env) {
        return;
    }
    markObject((Obj *)env);
}


void collect(Interpreter *interpreter) {
    // mark(interpreter->env);
    // markTable(&interpreter->strings);
    // sweep(&interpreter->objects);
}
