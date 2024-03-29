#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "register.h"
#include "value.h"
#include "types.h"

#define TABLE_MAX_LOAD 0.75

void initRegister(Register* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeRegister(Register* table) {
    ARR_FREE(Entry, table->entries, table->capacity);
    initRegister(table);
}

static Entry* findEntry(Entry* entries, int capacity, String* key) {
    uint32_t index = key->hash & (capacity - 1);
    Entry* tombstone = NULL;

    for (;;) {

        Entry* entry = &entries[index];
        if (entry->key == NULL) {
            if (IS_NULL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (strcmp(entry->key->chars, key->chars) == 0) return entry;

        index = (index + 1) & (capacity - 1);
    }
}

bool registerGet(Register* table, String* key, Value* value) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(Register* table, int capacity) {
    Entry* entries = ALLOC(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    ARR_FREE(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool registerSet(Register* table, String* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = NEXT_SIZE(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;

    if (isNewKey && IS_NULL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    #if __DBG_REGISTER
        printf("set %s with value ", key->chars);
        printValue(value);
        printf("\n");
    #endif
    return isNewKey;
}

bool registerDelete(Register* table, String* key) {
    if (table->count == 0) return false;

    // Find the entry.
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void registerAddAll(Register* from, Register* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
            registerSet(to, entry->key, entry->value);
        }
    }
}

String* registerFindString(
        Register* reg,
        const char* chars,
        int length,
        uint32_t hash) {
    if (reg->count == 0) return NULL;

    uint32_t index = hash & (reg->capacity - 1);
    for (;;) {
        Entry* entry = &reg->entries[index];
        if (entry->key == NULL) {
            if (IS_NULL(entry->value)) return NULL;
        } else if (entry->key->length == length &&
                   entry->key->hash == hash &&
                   memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }

        index = (index + 1) & (reg->capacity - 1);
    }
}

void registerRemoveWhite(Register* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->ptr.__gcFree) {
            registerDelete(table, entry->key);
        }
    }
}

void markRegister(Register* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        __gcTargetPtr((Ptr*)entry->key);
        __gcTargetValue(entry->value);
    }
}
