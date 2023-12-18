#ifndef sqript_chunk_h
#define sqript_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_EXPORT,
    OP_IMPORT,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_BITWISE_AND,
    OP_BITWISE_OR,
    OP_BITWISE_XOR,
    OP_BITWISE_NOT,
    OP_INCREMENT,
    OP_DECREMENT,
    OP_NOT,
    OP_NEGATE,
    OP_REF,
    OP_ARRAY,
    OP_ARRAY_GET,
    OP_ARRAY_SET,
    OP_ARRAY_ADD,
    OP_ARRAY_REMOVE,
    OP_PRINT,
    OP_PRINT_EXPR,
    OP_TYPEOF,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD
} OpCode;

typedef struct {
    int count;
    int capacity;
    Byte* code;
    int* lines;
    ValueArray constants;
} Segment;

void initSegment(Segment* s);
void freeSegment(Segment* s);

void writeSegment(Segment* s, Byte Byte, int line);
int registerConstant(Segment* s, Value value);

#endif
