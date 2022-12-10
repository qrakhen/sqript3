#include <stdio.h>

#include "digest.h"

void digestSegment(Segment* segment, const char* name)
{
    logSpam(name);
    for (int cursor = 0; cursor < segment->pos;) {
        cursor = digestInstruction(segment, cursor);
    }
}

int digestInstruction(Segment* segment, int position)
{
    logSpam(position);
    uint8_t instruction = segment->code[position];
    switch (instruction) {
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
    logSpam(name);
    return position + 1;
}

static int constantInstruction(const char* name, Segment* segment, int position) {
    uint8_t constant = segment->code[position + 1];
    logSpam(segment->constants.values[constant]);
    return position + 2;
}
