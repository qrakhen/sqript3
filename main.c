#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "segment.h"
#include "digest.h"
#include "log.h"
#include "runtime.h"

#define write writeSegment
#define init initSegment
#define clear clearSegment
#define digest digestSegment

int main(int argc, const char* argv[])
{
    //void* a;

    //DBG_printBytes(a);

    initRuntime();
    Segment seg;
    init(&seg);
    write(&seg, OP_CONSTANT, 0);
    write(&seg, 5, 0);
    write(&seg, OP_RETURN, 0);
    interpret(&seg);
    clearRuntime();
    clear(&seg);
    return 0;
}
