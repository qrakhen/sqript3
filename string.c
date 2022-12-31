#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "register.h"
#include "value.h"
#include "runner.h"
#include "string.h"

static PtrString* allocateString(char* chars, int length,
                                 uint32_t hash) {
    PtrString* string = ALLOCATE_PTR(PtrString, PTR_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(PTR_VAL(string));
    registerSet(&runner.strings, string, NULL_VAL);
    pop();

    return string;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (Byte)key[i];
        hash *= 16777619;
    }
    return hash;
}

PtrString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    PtrString* interned = registerFindString(&runner.strings, chars, length, hash);
    if (interned != NULL) {
        ARR_FREE(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

PtrString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    PtrString* interned = registerFindString(&runner.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOC(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

PtrString* subString(char* src, int m, int n) {
    int len = n - m;
    char* dest = (char*)malloc(sizeof(char) * (len + 1));
    strncpy(dest, (src + m), len);
    return copyString(dest, len);
}
