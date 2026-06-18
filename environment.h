#ifndef leaf_environment_h
#define leaf_environment_h


#include "table.h"


typedef struct Environment {
    struct Environment *previous;
    Table table;
} Environment;


Environment *makeEnvironment();
void freeEnvironment(Environment *env);


#endif
