#ifndef sqript_debug_h
#define sqript_debug_h

#include "segment.h"

void __dbgDissect(Segment* segment, const char* name);
int __dbgDissectOp(Segment* segment, int offset);

#endif
