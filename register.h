#ifndef sqript_register_h
#define sqript_register_h

#include "common.h"
#include "value.h"

typedef struct {
    String* key;
    Value value;
    TypeMode mode;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Register;

void initRegister(Register* table);
void freeRegister(Register* table);
bool registerGet(Register* table, String* key, Value* value);
bool registerSet(Register* table, String* key, Value value);
bool registerDelete(Register* table, String* key);
void registerAddAll(Register* from, Register* to);
String* registerFindString(Register* table, const char* chars,
                           int length, uint32_t hash);

void registerRemoveWhite(Register* table);
void markRegister(Register* table);

#endif
