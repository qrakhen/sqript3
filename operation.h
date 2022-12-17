#ifndef sqript_operation_h
#define sqript_operation_h

#include "common.h"

typedef enum {
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_SET,
    OP_CONSTANT,
    OP_RETURN,
} OpCode;

#endif
