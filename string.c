#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "register.h"
#include "value.h"
#include "string.h"
#include "runner.h"
#include "list.h"
#include "native.h"

#define EMPTY_STRING (makeString("", 0))

typedef uint32_t Hash;

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

void registerNativeMethods() {
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

int getCharLength(char* chars, char extraTerminator) {
    int i = 0;
    while (*(chars + i) != '\0' && *(chars + i) != extraTerminator) {
        i++;
    }
    return i;
}

String* subString(String* str, int from, int length) {
    if (length == -1) length = str->length - from;
    else if (from >= str->length) return EMPTY_STRING;
    return makeString(str->chars + from, length);
}

PtrArray* splitString(String* str, String* split) {
    char* start = str->chars;
    int pos = 0, last = 0;

    List* list = listCreate(T_PTR_STRING);
    while (start[pos] != '\0' && pos <= str->length) {
        if (start[pos] == split->chars[0]) {
            listPush(list, PTR_VAL(subString(str, last, pos - last)));
            last = ++pos;
        } else if (pos == str->length - 1) {
            listPush(list, PTR_VAL(subString(str, last, str->length - last)));
            break;
        }
        pos++;
    }

    return listToArray(list);
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

/*
 * NATIVES
 */

Value native_StringLength(Value target, int argCount, Value* args) {
    return NUMBER_VAL(((String*)AS_STRING(target))->length);
}

Value native_StringSubString(Value target, int argCount, Value* args) {
    return PTR_VAL(
        subString(
            AS_STRING(target), 
            NATIVE_GET_ARG(args, 0, AS_NUMBER), 
            argCount > 1 ? NATIVE_GET_ARG(args, 1, AS_NUMBER) : -1));
}

Value native_StringIndexOf(Value target, int argCount, Value* args) {
    return stringIndexOf(
            AS_STRING(target),
            NATIVE_GET_ARG(args, 0, AS_STRING));
}

Value native_StringSplit(Value target, int argCount, Value* args) {
    return PTR_VAL(
        splitString(
            AS_STRING(target), 
            argCount > 0 ? NATIVE_GET_ARG(args, 0, AS_STRING) : NULL));
}
