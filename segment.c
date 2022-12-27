#include <stdlib.h>

#include "segment.h"
#include "memory.h"
#include "runner.h"

void initSegment(Segment* segment) {
    segment->count = 0;
    segment->capacity = 0;
    segment->code = NULL;
    segment->lines = NULL;
    initValueArray(&segment->constants);
}

void freeSegment(Segment* segment) {
    ARR_FREE(Byte, segment->code, segment->capacity);
    ARR_FREE(int, segment->lines, segment->capacity);
    freeValueArray(&segment->constants);
    initSegment(segment);
}

void writeSegment(Segment* segment, Byte value, int line) {
    if (segment->capacity < segment->count + 1) {
        int oldCapacity = segment->capacity;
        segment->capacity = NEXT_SIZE(oldCapacity);
        segment->code = ARR_RESIZE(Byte, segment->code, oldCapacity, segment->capacity);
        segment->lines = ARR_RESIZE(int, segment->lines, oldCapacity, segment->capacity);
    }

    segment->code[segment->count] = value;
    segment->lines[segment->count] = line;
    segment->count++;
}

int registerConstant(Segment* segment, Value value) {
    push(value);
    writeValueArray(&segment->constants, value);
    pop();
    return segment->constants.count - 1;
}
