#ifndef sqript_value_h
#define sqript_value_h

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t byte;

typedef void* protoVal;
typedef protoVal Integer;

typedef enum {
    T_BOOL,
    T_BYTE,
    T_NUMBER,
    T_INT,
    T_NULL,
    T_STRING,
    T_COLLECTION,
    T_OBJECT
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
} Value;

#define IS_BOOL(v)      ((v).type == T_BOOL)
#define IS_NIL(v)       ((v).type == T_NULL)
#define IS_NUMBER(v)    ((v).type == T_NUMBER)

#define AS_BOOL(v)      ((v).as.boolean)
#define AS_NUMBER(v)    ((v).as.number)

#define VAL_BOOL(v)     ((Value) { T_BOOL, { .boolean = v } })
#define VAL_NULL        ((Value) { T_NULL, { .number = 0 } })
#define VAL_NUMBER(v)   ((Value) { T_NUMBER, { .number = v } })

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
