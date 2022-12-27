#ifndef sqript_runner_h
#define sqript_runner_h

#include "object.h"
#include "register.h"
#include "value.h"

#define MAX_QALLS 64
#define STACK_MAX (MAX_QALLS * BYTE_MAX)

typedef struct {
    PtrQlosure* qlosure;
    Byte* ip;
    Value* slots;
} Qall;

typedef struct {
    Qall qalls[MAX_QALLS];
    int qc;

    Value stack[STACK_MAX];
    Value* cursor;

    Register globals;
    Register strings;

    PtrString* __initString;
    PtrPreval* __openPrevals;

    Ptr* pointers;
    size_t bAlloc;

    size_t __gcNext;
    int __gcCount;
    int __gcLimit;
    Ptr** __gcStack;
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
