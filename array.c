#include "array.h"
#include "memory.h"

PtrArray* createArray(int length, ValueType type) {
    Value* values = ALLOC(Value, length);
    for (int i = 0; i < length; i++)
        values[i] = NULL_VAL;

    PtrArray* arr = (PtrArray*)allocatePtr(sizeof(PtrArray), PTR_ARRAY);
    arr->length = length;
    arr->type = type;
    arr->values = values;
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

void arrayAppend(PtrArray* arr, Value value) {
    arr->values = ARR_RESIZE(Value, arr->values, arr->length, arr->length + 1);
    arr->length++;
    arr->values[arr->length - 1] = value;
}

void arrayInsert(PtrArray* arr, int index, Value value) {

}

Value arrayPop(PtrArray* arr) {
    Value v = arr->values[arr->length - 1];
    ARR_RESIZE(Value, arr->values, arr->length, arr->length - 1);
    return v;
}

PtrArray* arraySpan(PtrArray* arr, int from, int length) {
    if (length == -1) length = arr->length - from;
    PtrArray* span = createArray(length, arr->type);
    span->length = length;
    memcpy(span->values, arr->values, sizeof(Value) * length);
    return span;
}

void arrayRemoveAt(PtrArray* arr, int index) {
    arr->values = ARR_RESIZE(Value, arr->values, arr->length, arr->length - 1);
}

void arrayRemove(PtrArray* arr, Value value) {

}

void freeArray(PtrArray* array) {
    
}

int arrayLength(PtrArray* arr) {
    return arr->length;
}

Value native_ArrayAppend(Value target, int argCount, Value* args) {
    arrayAppend(AS_ARRAY(target), args[0]);
    return NULL_VAL;
}
