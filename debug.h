#ifndef sqript_debug_h
#define sqript_debug_h

#include "segment.h"

void disassembleChunk(Segment* chunk, const char* name);
int disassembleInstruction(Segment* chunk, int offset);

#endif
