#ifndef sqript_table_h
#define sqript_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Register;

void initTable(Register* table);
void freeTable(Register* table);
bool tableGet(Register* table, ObjString* key, Value* value);
bool tableSet(Register* table, ObjString* key, Value value);
bool tableDelete(Register* table, ObjString* key);
void tableAddAll(Register* from, Register* to);
ObjString* tableFindString(Register* table, const char* chars,
                           int length, uint32_t hash);

void tableRemoveWhite(Register* table);
void markTable(Register* table);

#endif
