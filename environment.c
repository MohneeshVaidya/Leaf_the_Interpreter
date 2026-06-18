#include <stddef.h>


#include "environment.h"
#include "memory.h"
#include "table.h"
#include "value.h"


Environment *makeEnv() {
    Environment *env = REALLOCATE(NULL, 1, Environment);
    env->previous = NULL;
    initTable(&env->table);
    return env;
}


void freeEnv(Environment *env) {
    env->previous = NULL;
    freeTable(&env->table);
    FREE(env, Environment);
}


bool envAdd(Environment *env, ObjString *name, Value value) {
    if (tableGet(&env->table, name, NULL)) {
        return false;
    }
    return tableSet(&env->table, name, value);
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
