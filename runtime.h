#ifndef sqript_runtime_h
#define sqript_runtime_h

#include "segment.h"

typedef struct {
    Segment* segment;
    byte* ip;
} Runtime;

typedef enum {
    OK,
    ERR_COMPILE,
    ERR_RUNTIME
} InterpretResult;

void initRuntime();
void clearRuntime();
InterpretResult interpret(Segment* segment);
static InterpretResult run();

#endif
