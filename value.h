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
typedef struct String String;

typedef enum {
    T_ANY = 0,
    T_NULL = 1,
    T_BOOL = 2,
    T_BYTE = 4,
    T_INT = 8,
    T_DEC = 16,
    T_NUMBER = T_INT | T_DEC,

    T_PTR = 1024,
    T_PTR_METHOD = T_PTR | 1,
    T_PTR_QLASS = T_PTR | 2,
    T_PTR_QLOSURE = T_PTR | 4,
    T_PTR_FUNQ = T_PTR | 8,
    T_PTR_INSTANCE = T_PTR | 16,
    T_PTR_ARRAY = T_PTR | 32,
    T_PTR_LIST = T_PTR | 64,
    T_PTR_NATIVE = T_PTR | 128,
    T_PTR_STRING = T_PTR | 256,
    T_PTR_PREVAL = T_PTR | 512
} ValueType;

typedef enum {
    TM_DYN,
    TM_STRICT
} TypeMode;

struct Value {
    ValueType type;
    TypeMode mode;
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
#define IS_BYTE(v)          ((v).type == T_BYTE)
#define IS_NUMBER(v)        ((v).type == T_NUMBER)
//#define IS_INT(v)           ((v).type == T_INT)
#define MAYBE_INT(v)        (IS_NUMBER(v) && (abs(ceil(AS_NUMBER(v)) - floor(AS_NUMBER(v))) == 0))
#define IS_PTR(v)           ((v).type == T_PTR)
#define IS_ANY(v)           ((v).type == T_ANY)

#define AS_PTR(value)       ((value).as.ptr)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_BYTE(value)      ((value).as.byte)
#define AS_NUMBER(value)    ((value).as.number)
//#define AS_INT(value)       ((value).as.integer)

#define BOOL_VAL(value)   ((Value){ T_BOOL,     TM_DYN, { .boolean = value }})
#define NULL_VAL          ((Value){ T_NULL,     TM_DYN, { .ptr = NULL }})
#define BYTE_VAL(value)   ((Value){ T_BYTE,     TM_DYN, { .byte = value }})
#define NUMBER_VAL(value) ((Value){ T_NUMBER,   TM_DYN, { .number = value }})
//#define INT_VAL(value)    ((Value){ T_INT,      TM_DYN, { .integer = value }})
#define PTR_VAL(object)   ((Value){ T_PTR,      TM_DYN, { .ptr = (Ptr*)object }})

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
