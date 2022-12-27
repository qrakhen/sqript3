#ifndef sqript_value_h
#define sqript_value_h

#include <string.h>

#include "common.h"

typedef bool Bool;
typedef uint8_t Byte;
typedef int64_t Int;
typedef double Number;

typedef struct Value Value;
typedef struct Ptr Ptr;
typedef struct PtrString PtrString;

typedef enum {
    T_ANY,
    T_NULL,
    T_BOOL,
    T_BYTE,
    T_NUMBER,
    T_INTEGER,
    T_PTR,
} ValueType;

struct Value {
    ValueType type;
    union {
        Bool boolean;
        Byte byte;
        Int integer;
        Number number;
        Ptr* ptr;
    } as;
};

#define IS_BOOL(v)          ((v).type == T_BOOL)
#define IS_NULL(v)          ((v).type == T_NULL)
#define IS_NUMBER(v)        ((v).type == T_NUMBER)
#define IS_INTEGER(v)       ((v).type == T_INTEGER || IS_NUMBER(v) && (abs(ceil(AS_NUMBER(v)) - floor(AS_NUMBER(v))) == 0))
#define IS_OBJ(v)           ((v).type == T_PTR)
#define IS_ANY(v)           ((v).type == T_ANY)

#define AS_OBJ(value)       ((value).as.ptr)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)
#define AS_INTEGER(value)   ((value).as.Int)

#define BOOL_VAL(value)   ((Value){ T_BOOL,     { .boolean = value }})
#define NULL_VAL          ((Value){ T_NULL,     { .ptr = NULL }})
#define NUMBER_VAL(value) ((Value){ T_NUMBER,   { .number = value }})
#define INT_VAL(value)    ((Value){ T_INTEGER,  { .integer = value }})
#define OBJ_VAL(object)   ((Value){ T_PTR,      { .ptr = (Ptr*)object }})

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
char* valueToString(Value value);
void printValue(Value value);
void printType(Value value);

#endif
