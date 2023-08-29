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

typedef Value(*NativeFunq)(int argCount, Value* args);
typedef Value(*NativeMethod)(Value target, int argCount, Value* args);

typedef void(*QollectionFunq)(Value item, int index);
typedef Bool(*QollectionFilterFunq)(Value item, int index);
typedef Int(*QollectionSortFunq)(Value a, Value b);
typedef Value(*QollectionMutateFunq)(Value value);

typedef enum {
    VA_GLOBAL = 0,
    VA_PRIVATE = 1,
    VA_PROTECTED = 2,
    VA_INTERNAL = 4,
    VA_PUBLIC = 8,
    VA_STATIC = 16
} ValueAccess;

typedef enum {
    T_ANY = 0,
    T_NULL = 1,
    T_BOOL = 2,
    T_BYTE = 4,
    T_INT = 8,
    T_DEC = 16,
    T_NUMBER = T_INT | T_DEC,

    T_REF = 256,

    T_PTR = 512,
    T_PTR_METHOD = T_PTR | 1,
    T_PTR_QLASS = T_PTR | 2,
    T_PTR_QLOSURE = T_PTR | 4,
    T_PTR_FUNQ = T_PTR | 8,
    T_PTR_INSTANCE = T_PTR | 16,
    T_PTR_ARRAY = T_PTR | 32,
    T_PTR_NATIVE = T_PTR | 64,
    T_PTR_NATIVE_FUNQ = T_PTR_NATIVE | T_PTR_FUNQ,
    T_PTR_NATIVE_METHOD = T_PTR_NATIVE | T_PTR_METHOD,
    T_PTR_STRING = T_PTR | 128,
    T_PTR_PREVAL = T_PTR | 256,
    T_PTR_MODULE = T_PTR | 512
} ValueType;

typedef enum {
    TM_NONE = 0,
    TM_DYN = 1,
    TM_REF = 2,
    TM_STRICT = 4,
    TM_CONST = 8,
    TM_NULLABLE = 16
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
        Value* ref;
    } v;
};

typedef enum {
    TF_NONE = 0,
    TF_NUMERIC = 1,
    TF_STRING = 2,
    TF_PTR = 4,
    TF_PRIMITIVE = 8,
    TF_CLASS = 16
} TypeFlag;

typedef struct {
    ValueType type;
    TypeFlag flags;
} Type;

#define IS_ANY(v)           ((v).type == T_ANY)
#define IS_NULL(v)          ((v).type == T_NULL)
#define IS_BOOL(v)          ((v).type == T_BOOL)
#define IS_BYTE(v)          ((v).type == T_BYTE)
#define IS_INT(v)           ((v).type == T_INT)
#define IS_DEC(v)           ((v).type == T_DEC)
#define IS_NUMBER(v)        (((v).type & T_NUMBER) > 0)
#define MAYBE_INT(v)        (IS_NUMBER(v) && (abs(ceil(AS_NUMBER(v)) - floor(AS_NUMBER(v))) == 0))
#define IS_PTR(v)           (((int)(v).type & T_PTR) > 0)
#define IS_REF(v)           (((int)(v).type & T_REF) > 0)

#define AS_PTR(value)       ((value).v.ptr)
#define AS_REF(value)       ((value).v.ref)
#define AS_BOOL(value)      ((value).v.boolean)
#define AS_BYTE(value)      ((value).v.byte)
#define AS_NUMBER(value)    ((value).v.number)
#define AS_DEC(value)       ((value).v.number)
#define AS_INT(value)       ((value).v.integer)

#define BOOL_VAL(value)   ((Value){ T_BOOL,     TM_DYN, { .boolean = value }})
#define NULL_VAL          ((Value){ T_NULL,     TM_DYN, { .ptr = NULL }})
#define BYTE_VAL(value)   ((Value){ T_BYTE,     TM_DYN, { .byte = value }})
#define NUMBER_VAL(value) ((Value){ T_NUMBER,   TM_DYN, { .number = value }})
#define INT_VAL(value)    ((Value){ T_INT,      TM_DYN, { .integer = value }})
#define PTR_VAL(object)   ((Value){ T_PTR,      TM_DYN, { .ptr = (Ptr*)object }})
#define REF_VAL(value)    ((Value){ T_REF,      TM_DYN, { .ref = (Value*)&value }})

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
char* printType(Value value);

#endif
