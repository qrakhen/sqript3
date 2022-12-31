#ifndef sqript_list_h
#define sqript_list_h

#include <stdlib.h>

#include "value.h"
#include "array.h"

typedef struct List List;

struct List {
    int length;
    ValueType type;
    struct __Node* head;
};

List* createList(ValueType type);
void listPush(List* list, Value value);
Value listGet(List* list, int index);
Value listPop(List* list);
void listInsert(List* list, int index, Value value);
void listDelete(List* list, int index);
int listFind(List* list, Value value);
void freeList(List* list);
PtrArray* listToArray(List* list);

#endif