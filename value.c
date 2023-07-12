#include <stdio.h>
#include <string.h>
#include <math.h>

#include "object.h"
#include "memory.h"
#include "value.h"
#include "native.h"
#include "string.h"

static char* modeToString(TypeMode mode) {
    switch (mode) {
        case TM_NONE: return "none";
        case TM_DYN: return "dyn";
        case TM_STRICT: return "strict";
        case TM_CONST: return "const";
        case TM_REF: return "ref";
        default: return "unknown";
    }
}

static Value toString(Value target, int argc, Value* params) {

}

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = NEXT_SIZE(oldCapacity);
        array->values = ARR_RESIZE(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    ARR_FREE(Value, array->values, array->capacity);
    initValueArray(array);
}

Value getValue(Value value) {
    if (value.type == T_REF)
        return *value.v.ref;
    else
        return value;
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
        value.v.ptr->type;
    }
    return buffer;
}

void printValue(Value value) {
    #if __DBG_SHOW_FULL_INFO
        printf("[%s]:", modeToString(value.mode));
        printf("<%s> ", printType(value));
    #endif

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
        case T_REF:
            printf("&");
            //Value x = *AS_REF(value);
            printValue(*value.v.ref);
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

char* printType(Value value) {
    switch (value.type) {
        case T_ANY: return printf("any"); break;
        case T_BOOL:return ("bool"); break;
        case T_NULL:return ("null"); break;
        case T_BYTE:return ("byte"); break;
        case T_INT:return ("int"); break;
        case T_NUMBER:return ("number"); break;
        case T_REF:return ("ref<>"); break;
        case T_PTR:
            switch (AS_PTR(value)->type) {
                case PTR_METHOD:return ("method"); break;
                case PTR_NATIVE_METHOD:return ("method(native)"); break;
                case PTR_QLASS:return AS_QLASS(value)->name->chars; break;
                case PTR_QLOSURE:return ("qlosure"); break;
                case PTR_FUNQ:return ("funqtion"); break;
                case PTR_OBJEQT:return AS_OBJEQT(value)->qlass->name->chars; break;
                case PTR_ARRAY:return "array"; //AS_ARRAY(value)->length; break;
                case PTR_NATIVE_FUNQ:return ("native"); break;
                case PTR_STRING:return ("string"); break;
                case PTR_PREVAL:return ("preval"); break;
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
        case T_PTR:     return AS_PTR(a) == AS_PTR(b);
        default:        return false;
    }
    #endif
}
