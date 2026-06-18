#include <stddef.h>

#include "table.h"
#include "memory.h"
#include "object.h"
#include "value.h"


#define INITIAL_CAPACITY (256)
#define LOAD_FACTOR (0.75)


void initTable(Table *table) {
    table->capacity = INITIAL_CAPACITY;
    table->count = 0;
    table->array = REALLOCATE(NULL, table->capacity, Entry);

    for (int i = 0; i < table->capacity; i++) {
        table->array[i].key = NULL;
        table->array[i].value = NIL_VALUE;
    }
}


void freeTable(Table *table) {
    table->capacity = 0;
    table->count = 0;
    table->array = FREE(table->array, Entry);
}


static bool isTombStone(const Entry *entry) {
    return entry->key == NULL && IS_BOOLEAN(entry->value);
}


static bool isEmpty(const Entry *entry) {
    return entry->key == NULL && IS_NIL(entry->value);
}


static void ensureCapacity(Table *table) {
    if (table->capacity * LOAD_FACTOR > table->count) {
        return;
    }
    int newCapacity = GROW_CAPACITY(table->capacity);
}


static Entry *findEntry(Entry *entries, int capacity, const ObjString *key, bool isLookupMode) {
    int index = key->hash % capacity;

    Entry *tombStone = NULL;
    for (int i = 0; i < capacity; i++) {
        Entry *entry = entries + index;

        if (tombStone == NULL && isTombStone(entry)) {
            tombStone = entry;
        }
        if ((isEmpty(entry) && isLookupMode) || equalStrings(entry->key, key)) {
            return entry;
        }
        if (isEmpty(entry)) {
            return tombStone ? tombStone : entry;
        }

        index = (index + 1) % capacity;
    }
    return NULL;
}


void tableSet(Table *table, ObjString *key, Value value) {
    ensureCapacity(table);

    Entry *entry = findEntry(table->array, table->capacity, key, false);
    if (isEmpty(entry)) {
        table->count++;
    }
    entry->key = key;
    entry->value = value;
}


bool tableGet(const Table *table, ObjString *key, Value *value) {
    if (table->count == 0) return false;

    Entry *entry = findEntry(table->array, table->capacity, key, true);
    if (entry == NULL || entry->key == NULL) return false;

    *value = entry->value;

    return true;
}


void tableRemove(Table *table, ObjString *key) {
    if (table->count == 0) return;

    Entry *entry = findEntry(table->array, table->capacity, key, true);
    if (entry == NULL || entry->key == NULL) return;

    entry->key = NULL;
    entry->value = BOOLEAN_VALUE(false);
}
