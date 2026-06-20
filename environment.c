#include <stddef.h>


#include "environment.h"
#include "memory.h"
#include "table.h"


Environment *makeEnv(Interpreter *interpreter) {
    Environment *env = REALLOCATE(NULL, 0, 1, interpreter, Environment);

    env->meta.type = OBJ_ENVIRONMENT;
    env->meta.next = interpreter->objects;
    interpreter->objects = (Obj *)env;
    env->meta.isMarked = false;

    env->previous = NULL;
    env->saved = NULL;
    initTable(&env->table, interpreter);
    return env;
}


void freeEnv(Environment *env) {
    env->previous = NULL;
    env->saved = NULL;
    freeTable(&env->table);
}


bool envAdd(Environment *env, ObjString *name, Value value, Interpreter *interpreter) {
    if (tableGet(&env->table, name, NULL)) {
        return false;
    }
    return tableSet(&env->table, name, value, interpreter);
}


bool envSet(Environment *env, ObjString *name, Value value, Interpreter *interpreter) {
    if (tableGet(&env->table, name, NULL)) {
        tableSet(&env->table, name, value, interpreter);
        return true;
    }
    if (env->previous) {
        return envSet(env->previous, name, value, interpreter);
    }
    return false;
}


bool envGet(Environment *env, ObjString *name, Value *value) {
    if (tableGet(&env->table, name, value)) {
        return true;
    }
    if (env->previous) {
        return envGet(env->previous, name, value);
    }
    return false;
}
