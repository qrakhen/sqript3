#include <stdio.h>

#include "common.h"
#include "runtime.h"
#include "digest.h"

Runtime rt;

void initRuntime() {

}

void clearRuntime() {

}

InterpretResult interpret(Segment* segment) {
    rt.segment = segment;
    rt.ip = rt.segment->code;
    return run();
}

static InterpretResult run() {
    #define RBYTE() (*rt.ip++)
    #define CONST() (rt.segment->constants.values[RBYTE()])

    do {
        #ifdef DEBUG_MODE
        digestInstruction(rt.segment, (int)(rt.ip = rt.segment->code));
        #endif
        byte ix;
        switch (ix = RBYTE()) {
        case OP_CONSTANT:
            Value c = CONST();
            printValue(c);
            break;
        case OP_RETURN:
            return OK;
        }
    } while(true);

    #undef INSTR
    #undef CONST
}
