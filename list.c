/**
 * Sqript Native Library
 * Linked List
 */

#include <stdlib.h>

#include "memory.h"
#include "object.h";
#include "list.h";

typedef struct __Node __Node;

struct __Node {
    Value value;
    __Node* next;
};

static __Node* __newNode() {
    __Node* node = ALLOC(__Node, 1);
    node->next = NULL;
    node->value = NULL_VAL;
    return node;
}

List* listCreate(ValueType type) {
    List* list = ALLOCATE_PTR(List, PTR_LIST);
    list->head = __newNode();
    list->length = 0;
    list->type = type;
    return list;
}

Value listGet(List* list, int index) {
    if (index >= list->length)
        return NULL_VAL;

    __Node* cur = list->head;
    int pos = 0;
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
    cur->next = __newNode();
    cur->value = value; 
    list->length++;
}

Value listPop(List* list) {
    list->length--;
}

void listInsert(List* list, int index, Value value) {
    list->length++;
}

void listDelete(List* list, int index) {
    list->length--;
}

int listFindIndex(List* list, Value value) {
    return -1;
}

void freeList(List* list) {
    // yea well... free?
    reallocate(list, list->length * sizeof(List), 0);
}

PtrArray* listToArray(List* list) {
    __Node* cur = list->head;
    PtrArray* arr = createArray(list->length, list->type);
    for (int i = 0; i < list->length; i++) {
        arr->values[i] = cur->value;
        cur = cur->next;
    }
    return arr;
}
