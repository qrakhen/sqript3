#ifndef sqript_operation_h
#define sqript_operation_h

#include "common.h"

typedef enum {
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_SET,
    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_XOR,
    OP_BITWISE_NOT,
    OP_CONSTANT,
    OP_RETURN,
} OpCode;

#endif
