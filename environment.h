#ifndef leaf_environment_h
#define leaf_environment_h


#include "table.h"


typedef struct Environment {
    struct Environment *previous;
    Table table;
} Environment;


Environment *makeEnv();
void freeEnv(Environment *env);
bool envAdd(Environment *env, ObjString *name, Value value);
bool envSet(Environment *env, ObjString *name, Value value);
bool envGet(Environment *env, ObjString *name, Value *value);


#endif
