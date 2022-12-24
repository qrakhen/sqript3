#ifndef sqript_list_h
#define sqript_list_h

#include <stdlib.h>

#include "value.h"

typedef struct {
    Value value;
    int index;
    struct Node* next;
} Node;

typedef struct {
    int length;
    Node* head;
    Node* current;
} List;

List* createList();
void listAdd(Value value);
void listInsert(int index, Value value);
void listDelete(int index, Value value);
Value listGet(int index);
int listAt(Value value);
void freeList(List* list);

#endif