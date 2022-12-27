#ifndef sqript_runner_h
#define sqript_runner_h

#include "object.h"
#include "register.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    PtrQlosure* closure;
    byte* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Register globals;
    Register strings;
    PtrString* initString;
    PtrPreval* openUpvalues;

    size_t bytesAllocated;
    size_t nextGC;
    Ptr* objects;
    int grayCount;
    int grayCapacity;
    Ptr** grayStack;
} Runner;

typedef enum {
    SQR_INTRP_OK,
    SQR_INTRP_ERROR_DIGEST,
    SQR_INTRP_ERROR_RUNTIME
} InterpretResult;

extern Runner runner;

void initRunner();
void freeRunner();

InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
