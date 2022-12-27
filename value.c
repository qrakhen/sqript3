#include <stdio.h>
#include <string.h>
#include <math.h>

#include "object.h"
#include "memory.h"
#include "value.h"
#include "array.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = NEXT_SIZE(oldCapacity);
        array->values = ARR_RESIZE(Value, array->values,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    ARR_FREE(Value, array->values, array->capacity);
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
    } else if (IS_PTR(value)) {
        printObject(value);
    }
    return buffer;
}

void printValue(Value value) {
    switch (value.type) {
        case T_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case T_NULL: printf("NULL"); break;
        //case T_INT:printf("%ld", AS_INT(value));
        case T_NUMBER: 
            if (MAYBE_INT(value))
                printf("%0.lf", AS_NUMBER(value));
            else
                printf("%g", AS_NUMBER(value));
            break;
        case T_PTR: 
            if (matchPtrType(value, PTR_ARRAY)) {
                printf("[");
                for (int i = 0; i < AS_ARRAY(value)->length; i++) {
                    printValue(AS_ARRAY(value)->values[i]);
                    if (i < AS_ARRAY(value)->length - 1)
                        printf(", ");
                }
                printf("]");
                break;
            } else
                printObject(value); break;
    }
}

void printType(Value value) {
    switch (value.type) {
        case T_ANY: printf("any"); break;
        case T_BOOL: printf("bool"); break;
        case T_NULL: printf("null"); break;
        case T_BYTE: printf("byte"); break;
        case T_INT: printf("int"); break;
        case T_NUMBER: printf("number"); break;
        case T_PTR:
            switch (AS_OBJ(value)->type) {
                case PTR_METHOD: printf("method"); break;
                case PTR_QLASS: printf("qlass<%s>", AS_CLASS(value)->name->chars); break;
                case PTR_QLOSURE: printf("qlosure"); break;
                case PTR_FUNQ: printf("funqtion"); break;
                case PTR_INSTANCE: printf("instance<%s>", AS_INSTANCE(value)->klass->name->chars); break;
                case PTR_ARRAY: printf("array<any>[%d]", AS_ARRAY(value)->length); break;
                case PTR_NATIVE: printf("native"); break;
                case PTR_STRING: printf("string"); break;
                case PTR_PREVAL: printf("preval"); break;
            }
            break;
    }
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
        case T_BOOL:    return AS_BOOL(a) == AS_BOOL(b);
        case T_NULL:    return true;
        case T_NUMBER:  return AS_NUMBER(a) == AS_NUMBER(b);
        case T_PTR:     return AS_OBJ(a) == AS_OBJ(b);
        default:        return false;
    }
    #endif
}
