#ifndef sqript_string_h
#define sqript_string_h

#include "value.h"
#include "object.h"
#include "types.h"

#define IS_STRING(value)       matchPtrType(value, PTR_STRING)

#define AS_STRING(value)       ((PtrString*)AS_PTR(value))
#define AS_CSTRING(value)      (((PtrString*)AS_PTR(value))->chars)

struct PtrString {
    Ptr ptr;
    int length;
    char* chars;
    uint32_t hash;
};

PtrString* takeString(char* chars, int length);
PtrString* copyString(const char* chars, int length);

#endif
