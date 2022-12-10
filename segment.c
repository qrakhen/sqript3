#include <stdlib.h>

#include "segment.h"
#include "memory.h"

void initSegment(Segment* segment)
{
    segment->pos = 0;
    segment->size = 0;
    segment->code = NULL;
    initValueArray(&segment->constants);
}

void clearSegment(Segment* segment)
{
    FREE_ARRAY(uint8_t, segment->code, segment->size);
    clearValueArray(&segment->constants);
    initSegment(segment);
}

void writeSegment(Segment* segment, uint8_t byte)
{
    if (segment->size < segment->pos + 1) {
        int _limit = segment->size;
        segment->size = SCALE_LIMIT(_limit);
        segment->code = SCALE_ARRAY(uint8_t, segment->code, _limit, segment->size);
    }

    segment->code[segment->pos] = byte;
    segment->pos++;
}

int registerConstant(Segment* segment, Value value) {
    writeValueArray(&segment->constants, value);
    return segment->constants.size - 1;
}
