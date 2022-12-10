#ifndef sqript_log_h
#define sqript_log_h

#include "segment.h"

void digestSegment(Segment* segment, const char* name);
int digestInstruction(Segment* segment, int position);
static int basicInstruction(const char* name, int position);
static int constantInstruction(const char* name, Segment* segment, int position);

#endif
