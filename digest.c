#include <stdio.h>

#include "digest.h"
#include "log.h"

void digestSegment(Segment* segment, const char* name)
{
    logSpam(name);
    for (int cursor = 0; cursor < segment->pos;) {
        cursor = digestInstruction(segment, cursor);
    }
}

int digestInstruction(Segment* segment, int position)
{
    printf("%04d ", position);
    uint8_t instruction = segment->code[position];
    switch (instruction) {
    case OP_NEG:
        return basicInstruction("OP_NEG", position);
    case OP_ADD:
        return basicInstruction("OP_ADD", position);
    case OP_SUB:
        return basicInstruction("OP_SUB", position);
    case OP_MUL:
        return basicInstruction("OP_MUL", position);
    case OP_DIV:
        return basicInstruction("OP_DIV", position);
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", segment, position);
    case OP_RETURN:
        return basicInstruction("OP_RETURN", position);
    default:
        logError("unknown instruction %d\n", instruction);
        return position + 1;
    }
}

static int basicInstruction(const char* name, int position) {
    printf("%s\n", name);
    return position + 1;
}

static int constantInstruction(const char* name, Segment* segment, int position) {
    uint8_t constant = segment->code[position + 1];
    printf("%s: ", name);
    printValue(segment->constants.values[constant]);
    printf("\n");
    return position + 2;
}
