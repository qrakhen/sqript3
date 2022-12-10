#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "segment.h"
#include "digest.h"
#include "log.h"

#define ws writeSegment
#define is initSegment
#define cs clearSegment
#define ds digestSegment

int main(int argc, const char* argv[])
{
    logInfo("zers");
    Segment s;
    is(&s);
    int c = registerConstant(&s, 36.72);
    ws(&s, OP_CONSTANT);
    ws(&s, c);
    ws(&s, OP_RETURN);
    ds(&s, "test 123");
    cs(&s);
    return 0;
}
