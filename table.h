#ifndef leaf_table_h
#define leaf_table_h

#include "forward.h"


void initTable(Table *table, Interpreter *interpreter);
void freeTable(Table *table);
bool tableSet(Table *table, ObjString *key, Value value, Interpreter *interpreter);
bool tableGet(const Table *table, ObjString *key, Value *value);
bool tableRemove(Table *table, ObjString *key);


#endif
