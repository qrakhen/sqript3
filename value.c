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
    printf(" %g ", value);
}
