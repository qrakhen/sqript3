#ifndef sqript_debug_h
#define sqript_debug_h

#include "segment.h"

void disassembleChunk(Segment* segment, const char* name);
int disassembleInstruction(Segment* segment, int offset);

#endif
