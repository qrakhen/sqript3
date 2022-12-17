#include <stdlib.h>

#include "segment.h"
#include "memory.h"

void initSegment(Segment* segment)
{
    segment->pos = 0;
    segment->size = 0;
    segment->code = NULL;
    segment->lines = NULL;
    initValueArray(&segment->constants);
}

void clearSegment(Segment* segment)
{
    FREE_ARRAY(uint8_t, segment->code, segment->size);
    FREE_ARRAY(int, segment->lines, segment->size);
    clearValueArray(&segment->constants);
    initSegment(segment);
}

void writeSegment(Segment* segment, uint8_t byte, int line)
{
    if (segment->size < segment->pos + 1) {
        int size = segment->size;
        segment->size = SCALE_LIMIT(size);
        segment->code = SCALE_ARRAY(uint8_t, segment->code, size, segment->size);
        segment->lines = SCALE_ARRAY(int, segment->lines, size, segment->size);
    }
    segment->code[segment->pos] = byte;
    segment->lines[segment->pos] = line;
    segment->pos++;
}

int registerConstant(Segment* segment, Value value)
{
    writeValueArray(&segment->constants, value);
    return segment->constants.pos - 1;
}
