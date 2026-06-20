#ifndef leaf_environment_h
#define leaf_environment_h


#include "forward.h"


#define IS_ENVIRONMENT(value)       (isObjType(value, OBJ_ENVIRONMENT))
#define AS_ENVIRONMENT(value)       ((Environment *)AS_OBJ(value))


Environment *makeEnv(Interpreter *interpreter);
void freeEnv(Environment *env);
bool envAdd(Environment *env, ObjString *name, Value value, Interpreter *interpreter);
bool envSet(Environment *env, ObjString *name, Value value, Interpreter *interpreter);
bool envGet(Environment *env, ObjString *name, Value *value);


#endif
