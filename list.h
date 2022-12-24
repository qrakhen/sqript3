#ifndef sqript_list_h
#define sqript_list_h

#include <stdlib.h>

#include "value.h"

typedef struct {
    Value value;
    struct Node* next;
} Node;

struct List {
    int length;
    ValueType type;
    Node* head;
    Node* current;
};

List* createList(ValueType type);
void listPush(Value value);
Value listPop();
void listInsert(int index, Value value);
void listDelete(int index, Value value);
Value listGet(int index);
int listFind(Value value);
void freeList(List* list);

#endif