#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "object.h"
#include "register.h"
#include "value.h"
#include "runner.h"
#include "string.h"
#include "array.h"

#define EMPTY_STRING (makeString("", 0))

typedef uint32_t Hash;
typedef struct SplitEntry SplitEntry;

struct SplitEntry {
    String* str;
    SplitEntry* next;
};

static String* allocateString(char* chars, int length, Hash hash) {
    String* string = ALLOCATE_PTR(String, PTR_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(PTR_VAL(string));
    registerSet(&runner.strings, string, NULL_VAL);
    pop();

    return string;
}

static Hash hash(const char* key, int length) {
    Hash hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (Byte)key[i];
        hash *= 16777619;
    }
    return hash;
}

String* takeString(char* str, int len) {
    Hash h = hash(str, len);
    String* interned = registerFindString(&runner.strings, str, len, h);
    if (interned != NULL) {
        ARR_FREE(char, str, len + 1);
        return interned;
    }

    return allocateString(str, len, h);
}

String* makeString(const char* chars, int length) {
    Hash h = hash(chars, length);
    String* interned = registerFindString(&runner.strings, chars, length, h);
    if (interned != NULL) return interned;

    char* heapChars = ALLOC(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, h);
}

String* subString(String* str, int from, int length) {
    if (length < 1 || from >= str->length) return EMPTY_STRING;
    return makeString(str->chars + from, length);
}

static __push(SplitEntry* e, String* str) {
    SplitEntry* cur = e;

}

PtrArray* splitString(String* str, String* split) {
    char* start = str->chars;
    int 
        count = 0, 
        pos = 0, 
        last = 0;

    SplitEntry head;
    head.str = NULL;
    head.next = NULL;
    SplitEntry* cur = &head;
    while (start[pos] != '\0' && pos <= str->length) {
        if (start[pos] == split->chars[0]) {
            // @todo use linked list
            cur->str = subString(str, last, pos - last);
            cur->next = ALLOC(SplitEntry, 1);
            cur = cur->next;
            last = ++pos; // skip split char
            count++;
        } else if (pos == str->length - 1) {
            cur->str = subString(str, last, str->length - last);
            count++;
            break;
        }
        pos++;
    }

    cur = &head;
    PtrArray* arr = createArray(count, T_PTR);
    for (int i = 0; i < count; i++) {
        arr->values[i] = PTR_VAL(cur->str);
        cur = cur->next;
    }
    return arr;
}

Value stringIndexOf(String* str, String* needle) {
    char* start = str->chars;
    int pos = 0;

    while (start[pos] != '\0' && pos <= str->length - needle->length) {
        for (int i = 0; i < needle->length; i++) {
            if (start[pos + i] != needle->chars[i])
                break;
            if (i == needle->length - 1)
                return NUMBER_VAL(pos);
        }
        pos++;
    }

    return NUMBER_VAL(-1);
}