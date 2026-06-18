#ifndef leaf_table_h
#define leaf_table_h


#include "value.h"


typedef struct Entry {
    ObjString *key;
    Value value;
} Entry;


typedef struct Table {
    int capacity;
    int count;
    Entry *array;
} Table;


void initTable(Table *table);
void freeTable(Table *table);
bool tableSet(Table *table, ObjString *key, Value value);
bool tableGet(const Table *table, ObjString *key, Value *value);
bool tableRemove(Table *table, ObjString *key);


#endif
