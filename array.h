#ifndef sqript_array_h
#define sqript_array_h

#include "value.h"
#include "object.h"

#define IS_ARRAY(value) matchPtrType(value, PTR_ARRAY)
#define AS_ARRAY(value) ((PtrArray*)AS_PTR(value))

typedef struct {
    Ptr ptr;
    int length;
    ValueType type;
    Value* values;
} PtrArray;

PtrArray* createArray(int length, ValueType type);
Value arrayGet(PtrArray* array, int index);
bool arraySet(PtrArray* array, int index, Value value);
void freeArray(PtrArray* array);

int arrayLength(PtrArray* array);

#endif