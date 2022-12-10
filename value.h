#ifndef sqript_type_h
#define sqript_type_h

#include <stdint.h>

typedef uint8_t constant;
typedef uint8_t byte;
typedef double Value;

typedef struct {
    int pos;
    int size;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void clearValueArray(ValueArray* array);
void printValue(Value value);

#endif
