#ifndef sqript_value_h
#define sqript_value_h

#include <stdint.h>

typedef uint8_t byte; // brauchma immer
typedef double Value; // eeeeigentlich ja long oder so? für bitwise ops

typedef void* protoVal;
typedef protoVal Integer;

typedef enum {
    T_Boolean,
    T_Integer,
    T_Number,
    T_String
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

typedef struct {
    byte value;

} Boolean;

//typedef int Integer;
typedef float Number;
typedef char* String;

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void clearValueArray(ValueArray* array);
void printValue(Value value);

#endif
