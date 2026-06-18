#include <stddef.h>


#include "environment.h"
#include "memory.h"


Environment *makeEnvironment() {
    Environment *env = REALLOCATE(NULL, 1, Environment);
    env->previous = NULL;
    initTable(&env->table);
    return env;
}


void freeEnvironment(Environment *env) {
    env->previous = NULL;
    freeTable(&env->table);
    FREE(env, Environment);
}
