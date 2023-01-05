#ifndef sqript_array_h
#define sqript_array_h

#include "value.h"
#include "object.h"

#define IS_ARRAY(value) matchPtrType(value, PTR_ARRAY)
#define AS_ARRAY(value) ((PtrArray*)AS_PTR(value))

typedef struct {
    Ptr ptr;
    int length;
    bool fixedSize;
    ValueType type;
    Value* values;
} PtrArray;

PtrArray* createArray(int length, ValueType type);
Value arrayGet(PtrArray* array, int index);
bool arraySet(PtrArray* array, int index, Value value);
void freeArray(PtrArray* array);

PtrArray* arraySpan(PtrArray* arr, int from, int length);
void arrayAppend(PtrArray* array, Value value);
void arrayRemoveAt(PtrArray* arr, int index);
void arrayRemove(PtrArray* arr, Value value);
void arrayInsert(PtrArray* arr, int index, Value value);
int arrayLength(PtrArray* array);
void freeArray(PtrArray* array);

#endif