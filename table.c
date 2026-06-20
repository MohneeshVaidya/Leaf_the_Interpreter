#include <stddef.h>

#include "table.h"
#include "memory.h"
#include "object.h"
#include "value.h"


#define INITIAL_CAPACITY (8)
#define LOAD_FACTOR (0.75)


#define TOMBSTONE_VALUE     ((Value){ VALUE_TOMBSTONE, { .number = 0 } })
#define IS_TOMBSTONE(value) ((value).type == VALUE_TOMBSTONE)


static void initEntries(Entry *entries, int capacity) {
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VALUE;
    }
}


void initTable(Table *table) {
    table->capacity = INITIAL_CAPACITY;
    table->count = 0;
    table->array = REALLOCATE(NULL, table->capacity, Entry);
    initEntries(table->array, table->capacity);
}


void freeTable(Table *table) {
    table->capacity = 0;
    table->count = 0;
    table->array = FREE(table->array, Entry);
}


static bool isTombStone(const Entry *entry) {
    return entry->key == NULL && IS_TOMBSTONE(entry->value);
}


static bool isEmpty(const Entry *entry) {
    return entry->key == NULL && IS_NIL(entry->value);
}


static Entry *findEntryForInsertion(Entry *entries, int capacity, const ObjString *key) {
    int index = key->hash % capacity;

    Entry *tombStone = NULL;
    for (int i = 0; i < capacity; i++) {
        Entry *entry = entries + index;

        if (!tombStone && isTombStone(entry)) {
            tombStone = entry;
        }
        if (isEmpty(entry)) {
            return tombStone ? tombStone : entry;
        }
        if (equalStrings(entry->key, key)) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
    return NULL;
}


static Entry *findEntryForLookup(Entry *entries, int capacity, const ObjString *key) {
    int index = key->hash % capacity;

    for (int i = 0; i < capacity; i++) {
        Entry *entry = entries + index;

        if (isEmpty(entry) || equalStrings(entry->key, key)) {
            return entry;
        }

        index = (index + 1) % capacity;
    }

    return NULL;
}


static void ensureCapacity(Table *table) {
    if (table->count < table->capacity * LOAD_FACTOR) {
        return;
    }
    int capacity = GROW_CAPACITY(table->capacity);
    int count = 0;
    Entry *entries = REALLOCATE(NULL, capacity, Entry);
    initEntries(entries, capacity);

    for (int i = 0; i < table->capacity; i++) {
        const Entry *current = table->array + i;

        if (current->key != NULL) {
            Entry *entry = findEntryForInsertion(entries, capacity, current->key);

            entry->key = current->key;
            entry->value = current->value;

            count++;
        }
    }

    FREE(table->array, Entry);

    table->capacity = capacity;
    table->count = count;
    table->array = entries;
}


bool tableSet(Table *table, ObjString *key, Value value) {
    ensureCapacity(table);

    Entry *entry = findEntryForInsertion(table->array, table->capacity, key);
    if (entry) {
        if (isEmpty(entry)) {
            table->count++;
        }
        entry->key = key;
        entry->value = value;

        return true;
    }
    return false;
}


bool tableGet(const Table *table, ObjString *key, Value *value) {
    if (table->count == 0) return false;

    Entry *entry = findEntryForLookup(table->array, table->capacity, key);
    if (entry == NULL || entry->key == NULL) return false;

    if (value) {
        *value = entry->value;
    }

    return true;
}


bool tableRemove(Table *table, ObjString *key) {
    if (table->count == 0) return false;

    Entry *entry = findEntryForLookup(table->array, table->capacity, key);
    if (entry == NULL || entry->key == NULL) return false;

    entry->key = NULL;
    entry->value = TOMBSTONE_VALUE;

    return true;
}
