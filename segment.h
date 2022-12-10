#ifndef sqript_segment_h
#define sqript_segment_h

#include "common.h"
#include "operation.h"
#include "value.h"

typedef struct {
    int pos;
    int size;
    uint8_t* code;
    ValueArray constants;
} Segment;

void initSegment(Segment* segment);
void clearSegment(Segment* segment);
void writeSegment(Segment* segment, uint8_t byte);
int registerConstant(Segment* segment, Value value);

#endif
