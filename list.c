#include <stdlib.h>

#include "memory.h"
#include "object.h";
#include "list.h";

typedef struct __Node __Node;

struct __Node {
    Value value;
    __Node* next;
};

List* createList(ValueType type) {
    List* list = ALLOCATE_PTR(List, PTR_LIST);
    list->head = NULL;
    list->length = 0;
    list->type = type;
    return list;
}

Value listGet(List* list, int index) {
    __Node* cur = list->head;
    int pos = 0;
    if (index >= list->length)
        return NULL_VAL;

    while (pos < index) {
        cur = cur->next;
    }

    return cur->value;
}

void listPush(List* list, Value value) {
    __Node* cur = list->head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = ALLOC(__Node, 1);
    cur->next->value = value;
    cur->next->next = NULL;
    list->length++;
}

Value listPop(List* list);
void listInsert(List* list, int index, Value value);
void listDelete(List* list, int index);
int listFind(List* list, Value value);
void freeList(List* list);

PtrArray* listToArray(List* list) {
    __Node* cur = list->head;
    PtrArray* arr = createArray(list->length, list->type);
    for (int i = 0; i < list->length; i++) {
        arr->values[i] = cur->value;
        cur = cur->next;
    }
    return arr;
}
