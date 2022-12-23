#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "runtime.h"
#include "digest.h"
#include "digester.h"

Runtime rt;

static void resetStack() {
    rt.cursor = rt.stack;
}

static Value peek(int offset) {
    return rt.cursor[-1 - offset];
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = rt.ip - rt.segment->code - 1;
    int line = rt.segment->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);

    resetStack();
}

static InterpretResult run() {
    #define RBYTE() (*rt.ip++)
    #define CONST() (rt.segment->constants.values[RBYTE()])
    #define OPBIN(typeFn, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
                runtimeError("scalars expected"); \
                return ERR_RUNTIME; \
            } \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(typeFn(a op b)); \
        } while(false);

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
            case OP_NULL: push(VAL_NULL); break;
            case OP_TRUE: push(VAL_BOOL(true)); break;
            case OP_FALSE: push(VAL_BOOL(false)); break;

            case OP_ADD: OPBIN(VAL_NUMBER, +); break;
            case OP_SUB: OPBIN(VAL_NUMBER, -); break;
            case OP_MUL: OPBIN(VAL_NUMBER, *); break;
            case OP_DIV: OPBIN(VAL_NUMBER, /); break;

            case OP_NEG:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("operand must be scalar");
                    return ERR_RUNTIME;
                }
                push(VAL_NUMBER(-AS_NUMBER(pop())));
                break;

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

