#ifndef sqript_runtime_h
#define sqript_runtime_h

#include "segment.h"
#include "value.h"

#define STACK_MAX 255

typedef struct {
    Segment* segment;
    byte* ip;
    Value stack[STACK_MAX];
    Value* cursor;
} Runtime;

typedef enum {
    OK,
    ERR_COMPILE,
    ERR_RUNTIME,
    ERR_ARE_YOU_STOPED,
} InterpretResult;

void initRuntime();
void clearRuntime();
InterpretResult interpret(const char* source);

void push(Value value);
Value pop();

#endif
