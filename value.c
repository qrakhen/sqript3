#include <stdio.h>
#include <string.h>
#include <math.h>

#include "object.h"
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

char* valueToString(Value value) {
    char buffer[1024];
    if (IS_BOOL(value)) {
        sprintf(buffer, (AS_BOOL(value) ? "true" : "false"));
    } else if (IS_NULL(value)) {
        sprintf(buffer, "NULL");
    } else if (IS_NUMBER(value)) {
        sprintf(buffer, "%g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        printObject(value);
    }
    return buffer;
}

void printValue(Value value) {
    #ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "true" : "false");
    } else if (IS_NULL(value)) {
        printf("NULL");
    } else if (IS_INTEGER(value)) {
        printf("%0.lf", AS_NUMBER(value));
    } else if (IS_NUMBER(value)) {
        printf("%g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        printObject(value);
    }
    #else
    switch (value.type) {
        case T_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case T_NULL: printf("NULL"); break;
        case T_NUMBER: 
            if (IS_INTEGER(value))
                printf("%0.lf", AS_NUMBER(value));
            else
                printf("%g", AS_NUMBER(value));
            break;
        case T_OBJ: printObject(value); break;
    }
    #endif
}

bool valuesEqual(Value a, Value b) {
    #ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }
    return a == b;
    #else
    if (a.type != b.type) return false;
    switch (a.type) {
        case T_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case T_NULL:    return true;
        case T_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
            /* Strings strings-equal < Hash Tables equal
                case VAL_OBJ: {
                  ObjString* aString = AS_STRING(a);
                  ObjString* bString = AS_STRING(b);
                  return aString->length == bString->length &&
                      memcmp(aString->chars, bString->chars,
                             aString->length) == 0;
                }
             */
        case T_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
        default:         return false; // Unreachable.
    }
    #endif
}
