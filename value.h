#ifndef sqript_value_h
#define sqript_value_h

#include <string.h>

#include "common.h"

typedef uint8_t byte;
typedef uint16_t int16;
typedef uint32_t int32;
typedef uint64_t int64;
typedef struct Value Value;
typedef struct Obj Obj;
typedef struct Array Array;
typedef struct List List;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#define SIGN_BIT ((int64)0x8000000000000000)
#define QNAN     ((int64)0x7ffc000000000000)

#define TAG_NULL  1
#define TAG_FALSE 2
#define TAG_TRUE  3

typedef int64 Value;

#define IS_BOOL(value)      (((value) | 1) == TRUE_VAL)
#define IS_NULL(value)       ((value) == NULL_VAL)
#define IS_NUMBER(value)    (((value) & QNAN) != QNAN)
#define IS_INTEGER(value)    (IS_NUMBER(value) && (abs(ceil(AS_NUMBER(value)) - floor(AS_NUMBER(value))) == 0))
#define IS_OBJ(value) \
    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value)      ((value) == TRUE_VAL)
#define AS_NUMBER(value)    valueToNum(value)
#define AS_OBJ(value) \
    ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(b)     ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL       ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL        ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NULL_VAL         ((Value)(uint64_t)(QNAN | TAG_NULL))
#define NUMBER_VAL(num) numToValue(num)
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

typedef enum {
    T_ANY,
    T_BOOL,
    T_NULL,
    T_NUMBER,
    T_INTEGER,
    T_ARRAY,
    T_OBJ,
    T_PTR,
} ValueType;

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Value* ptr;
        Obj* obj;
    } as;
};

#define IS_BOOL(v)          ((v).type == T_BOOL)
#define IS_NULL(v)          ((v).type == T_NULL)
#define IS_NUMBER(v)        ((v).type == T_NUMBER)
#define IS_INTEGER(v)       (IS_NUMBER(v) && (abs(ceil(AS_NUMBER(v)) - floor(AS_NUMBER(v))) == 0))
#define IS_OBJ(v)           ((v).type == T_OBJ)
#define IS_PTR(v)           ((v).type == T_PTR)
#define IS_ARRAY(v)         ((v).type == T_ARRAY)
#define IS_ANY(v)           ((v).type == T_ANY)

#define AS_OBJ(value)       ((value).as.obj)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)
#define AS_PTR(value)       ((value).as.ptr)

#define BOOL_VAL(value)   ((Value){ T_BOOL,     { .boolean = value }})
#define NULL_VAL          ((Value){ T_NULL,     { .number = 0 }})
#define NUMBER_VAL(value) ((Value){ T_NUMBER,   { .number = value }})
#define OBJ_VAL(object)   ((Value){ T_OBJ,      { .obj = (Obj*)object }})
#define PTR_VAL(ptr)      ((Value){ T_PTR,      { .ptr = ptr }})

#endif

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

#endif
