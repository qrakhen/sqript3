#include <stdlib.h>

#include "value.h"
#include "memory.h"

void initValueArray(ValueArray* array)
{
    array->pos = 0;
    array->size = 0;
    array->values = NULL;
}

void clearValueArray(ValueArray* array)
{
    FREE_ARRAY(Value, array->values, array->size);
    initValueArray(array);
}

void writeValueArray(ValueArray* array, Value value)
{
    if (array->size < array->pos + 1) {
        int _limit = array->size;
        array->size = SCALE_LIMIT(_limit);
        array->values = SCALE_ARRAY(Value, array->values, _limit, array->size);
    }
    array->values[array->pos] = value;
    array->pos++;
}

void printValue(Value value)
{
    switch (value.type) {
        case T_BOOL: printf(AS_BOOL(value) ? "true" : "false"); break;
        case T_NULL: printf("null"); break;
        case T_NUMBER: printf("%g", AS_NUMBER(value)); break;
    }
}
