#ifndef sqript_array_h
#define sqript_array_h

#include "value.h"
#include "object.h"

#define IS_ARRAY(value) isObjType(value, OBJ_ARRAY)
#define AS_ARRAY(value) ((ObjArray*)AS_OBJ(value))

typedef struct {
    Obj obj;
    int length;
    ValueType type;
    Value* values;
} ObjArray;

ObjArray* createArray(int length, ValueType type);
Value arrayGet(int index);
void arraySet(int index, Value value);
void freeArray(ObjArray* array);

#endif