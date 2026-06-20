#ifndef leaf_object_h
#define leaf_object_h


#include "forward.h"
#include "expression.h"
#include "statement.h"


void printObj(const Obj *obj);

ObjString *internString(const char *chars,
                        int length,
                        Interpreter *interpreter);

ObjString *addStrings(const ObjString *a,
                      const ObjString *b,
                      Interpreter *interpreter);

bool equalStrings(const ObjString *a,
                  const ObjString *b);

ObjFn *makeObjFn(Parameter *parameters,
                 int arity,
                 Block *block,
                 Environment *closure,
                 Interpreter *interpreter);

ObjStruct *makeObjStruct(Block *block,
                         Environment *closure,
                         Interpreter *interpreter);

ObjStructValue *makeObjStructValue(Environment *context,
                                   Interpreter *interpreter);


#endif
