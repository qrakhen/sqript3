#ifndef sqript_array_h
#define sqript_array_h

#include "value.h"

struct Array {
	int length;
	ValueType type;
	Value* values;
};

Array* createArray(int length, ValueType type);
Value arrayGet(int index);
void arraySet(int index, Value value);
void freeArray(Array* array);

#endif