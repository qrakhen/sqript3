#include "array.h"

PtrArray* createArray(int length, ValueType type) {
    PtrArray* arr = (PtrArray*)allocatePtr(sizeof(PtrArray), PTR_ARRAY);
    arr->length = length;
    arr->type = type;
    return arr;
}

Value arrayGet(PtrArray* arr, int index) {
    if (index < 0 || index >= arr->length)
        return NULL_VAL;
    return arr->values[index];
}

bool arraySet(PtrArray* arr, int index, Value value) {
    if (index < 0 || index >= arr->length)
        return false;
    if (arr->type != T_ANY && value.type != arr->type)
        return false;
    arr->values[index] = value;
    return true;
}

void freeArray(PtrArray* array) {
    // hm...
}