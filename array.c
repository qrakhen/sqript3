#include "array.h"

ObjArray* createArray(int length, ValueType type);
Value arrayGet(int index);
void arraySet(int index, Value value);
void freeArray(ObjArray* array);