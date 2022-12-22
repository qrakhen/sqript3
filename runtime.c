#include <stdio.h>

#include "common.h"
#include "runtime.h"
#include "digest.h"
#include "digester.h"

Runtime rt;

static void resetStack() {
    rt.cursor = rt.stack;
}

static InterpretResult run() {
    #define RBYTE() (*rt.ip++)
    #define CONST() (rt.segment->constants.values[RBYTE()])
    #define OPBIN(op) do { Value b = pop(); Value a = pop(); push(a op b); } while(false);

    do {
        #ifdef DEBUG_MODE
        for (Value* v = rt.stack; v < rt.cursor; v++) {
            printf("[");
            printValue(*v);
            printf("]");
        }
        printf("\n");
        #endif
        byte ix;
        switch (ix = RBYTE()) {
            case OP_NEG: push(-pop()); break;
            case OP_ADD: OPBIN(+); break;
            case OP_SUB: OPBIN(-); break;
            case OP_MUL: OPBIN(*); break;
            case OP_DIV: OPBIN(/); break;
            // OP_AND, OR
            case OP_CONSTANT: {
                Value c = CONST();
                push(c);
                break;
            }
            case OP_RETURN: {
                printValue(pop());
                return OK;
            }
        }
    } while(true);

    #undef RBYTE
    #undef CONST
    #undef OPBIN
}

void initRuntime() {
    resetStack();
}

void clearRuntime() {

}

InterpretResult interpret(const char* source) {
    Segment seg;
    initSegment(&seg);
    if (!digest(source, &seg)) {
        clearSegment(&seg);
        return ERR_COMPILE;
    }
    rt.segment = &seg;
    rt.ip = rt.segment->code;
    InterpretResult result = run();
    clearSegment(&seg);
    return result;
}

void push(Value value) {
    printf(" push val: %g ", value);
    *(rt.cursor++) = value;
}

Value pop() {
    printf(" pop val: %g ", *(rt.cursor - 1));
    return *(--rt.cursor);
}

