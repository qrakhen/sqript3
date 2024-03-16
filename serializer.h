#include "value.h"
#ifndef sqript_serializer_h
#define sqript_serializer_h

typedef struct {
    char* chars;
    int length;
} CharArray;

CharArray serializeInstructions(Byte* bytes, int length);

#endif