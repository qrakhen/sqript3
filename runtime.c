#include <stdio.h>

#include "common.h"
#include "runtime.h"
#include "digest.h"

Runtime rt;

static void resetStack() {
    rt.cursor = rt.stack;
}

static InterpretResult run() {
    #define RBYTE() (*rt.ip++)
    #define CONST() (rt.segment->constants.values[RBYTE()])

    do {
        #ifdef DEBUG_MODE
        printf("schteck: ");
        for (Value* v = rt.stack; v < rt.cursor; v++) {
            printf("[");
            printValue(*v);
            printf("]");
        }
        printf("\n");
        digestInstruction(rt.segment, (int)(rt.ip = rt.segment->code));
        #endif
        byte ix;
        switch (ix = RBYTE()) {
        case OP_CONSTANT:
            Value c = CONST();
            push(c);
            break;
        case OP_RETURN:
            printValue(pop());
            return OK;
        }
    } while(true);

    #undef RBYTE
    #undef CONST
}

void initRuntime() {
    resetStack();
}

void clearRuntime() {

}

InterpretResult interpret(Segment* segment) {
    rt.segment = segment;
    rt.ip = rt.segment->code;
    return run();
}

void push(Value value) {
    *rt.cursor = value;
    rt.cursor++;
}

Value pop() {
    return *(--rt.cursor);
}

