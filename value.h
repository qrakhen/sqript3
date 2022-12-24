#ifndef sqript_value_h
#define sqript_value_h

#include <string.h>

#include "common.h"

typedef uint8_t byte;
typedef uint16_t int16;
typedef uint32_t int32;
typedef uint64_t int64;
typedef struct Obj Obj;
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
    T_BOOL,
    T_NULL,
    T_NUMBER,
    T_OBJ
} ValueType;


typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOL(value)    ((value).type == T_BOOL)
#define IS_NULL(value)    ((value).type == T_NULL)
#define IS_NUMBER(value)  ((value).type == T_NUMBER)
#define IS_OBJ(value)     ((value).type == T_OBJ)
#define IS_INTEGER(value) (IS_NUMBER(value) && (abs(ceil(AS_NUMBER(value)) - floor(AS_NUMBER(value))) == 0))

#define AS_OBJ(value)     ((value).as.obj)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define BOOL_VAL(value)   ((Value){ T_BOOL, {.boolean = value} })
#define NULL_VAL          ((Value){ T_NULL, {.number = 0} })
#define NUMBER_VAL(value) ((Value){ T_NUMBER, {.number = value} })
#define OBJ_VAL(object)   ((Value){ T_OBJ, {.obj = (Obj*)object} })

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
