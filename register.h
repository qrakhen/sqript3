#ifndef sqript_table_h
#define sqript_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
    bool typeStrict; // hier oder bei value???
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Register;

void initRegister(Register* table);
void freeRegister(Register* table);
bool registerGet(Register* table, ObjString* key, Value* value);
bool registerSet(Register* table, ObjString* key, Value value);
bool registerDelete(Register* table, ObjString* key);
void registerAddAll(Register* from, Register* to);
ObjString* registerFindString(Register* table, const char* chars,
                           int length, uint32_t hash);

void registerRemoveWhite(Register* table);
void markRegister(Register* table);

#endif
