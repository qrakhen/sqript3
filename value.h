#ifndef sqript_type_h
#define sqript_type_h

#include <stdint.h>

typedef uint8_t byte; // brauchma immer
typedef double Value; // eeeeigentlich ja long oder so? für bitwise ops

typedef enum {
    Boolean,
    Integer,
    Number,
    String
} ValueType;

typedef struct {
    int __v;
    ValueType type;
} _Value;

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
