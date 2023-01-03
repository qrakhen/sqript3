#ifndef sqript_string_h
#define sqript_string_h

#include "common.h"
#include "value.h"
#include "object.h"
#include "array.h"

#define IS_STRING(value)       matchPtrType(value, PTR_STRING)

#define AS_STRING(value)       ((String*)AS_PTR(value))
#define AS_CSTRING(value)      (((String*)AS_PTR(value))->chars)

struct String {
    Ptr ptr;
    int length;
    char* chars;
    uint32_t hash;
};

String* takeString(char* chars, int length);
String* makeString(const char* chars, int length);

String* subString(String* str, int from, int length);
Value stringIndexOf(String* str, String* needle);
PtrArray* splitString(String* str, String* split);

Value native_StringLength(Value target, int argCount, Value* args);
Value native_StringSubString(Value target, int argCount, Value* args);
Value native_StringIndexOf(Value target, int argCount, Value* args);
Value native_StringSplit(Value target, int argCount, Value* args);

#endif
