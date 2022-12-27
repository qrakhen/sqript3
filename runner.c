#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "digester.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "runner.h"
#include "console.h"

Runner runner;

static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
    runner.stackTop = runner.stack;
    runner.frameCount = 0;
    runner.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, C_COLOR_RED);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = runner.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &runner.frames[i];
        PtrFunq* function = frame->closure->function;
        size_t instruction = frame->ip - function->segment.code - 1;
        fprintf(stderr, "[line %d] in ", // [minus]
                function->segment.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    fprintf(stderr, C_COLOR_RESET);
    resetStack();
}

static void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    registerSet(&runner.globals, AS_STRING(runner.stack[0]), runner.stack[1]);
    pop();
    pop();
}

void initRunner() {
    resetStack();
    runner.objects = NULL;
    runner.bytesAllocated = 0;
    runner.nextGC = 1024 * 1024;

    runner.grayCount = 0;
    runner.grayCapacity = 0;
    runner.grayStack = NULL;

    initRegister(&runner.globals);
    initRegister(&runner.strings);

    runner.initString = NULL;
    runner.initString = copyString("init", 4);

    defineNative("clock", clockNative);
}

void freeRunner() {
    freeRegister(&runner.globals);
    freeRegister(&runner.strings);
    runner.initString = NULL;
    freeObjects();
}

void push(Value value) { *(runner.stackTop++) = value; }
Value pop() { return *(--runner.stackTop); }
static Value peek(int distance) { return runner.stackTop[-1 - distance]; }

static bool call(PtrQlosure* closure, int argCount) {
    if (argCount != closure->function->argc) {
        runtimeError("Expected %d arguments but got %d.",
                     closure->function->argc, argCount);
        return false;
    }

    if (runner.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &runner.frames[runner.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->segment.code;
    frame->slots = runner.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case PTR_METHOD: {
                PtrMethod* bound = AS_BOUND_METHOD(callee);
                runner.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount);
            }
            case PTR_QLASS: {
                PtrQlass* klass = AS_CLASS(callee);
                runner.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
                Value initializer;
                if (registerGet(&klass->methods, runner.initString,
                    &initializer)) {
                    return call(AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 arguments but got %d.",
                                 argCount);
                    return false;
                }
                return true;
            }
            case PTR_QLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case PTR_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, runner.stackTop - argCount);
                runner.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break;
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

static bool invokeFromClass(PtrQlass* klass, PtrString* name,
                            int argCount) {
    Value method;
    if (!registerGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount);
}

static bool invoke(PtrString* name, int argCount) {
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }

    PtrInstance* instance = AS_INSTANCE(receiver);

    Value value;
    if (registerGet(&instance->fields, name, &value)) {
        runner.stackTop[-argCount - 1] = value;
        return callValue(value, argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

static bool bindMethod(PtrQlass* klass, PtrString* name) {
    Value method;
    if (!registerGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    PtrMethod* bound = newBoundMethod(peek(0),
                                           AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

static PtrPreval* captureUpvalue(Value* local) {
    PtrPreval* prevUpvalue = NULL;
    PtrPreval* upvalue = runner.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    PtrPreval* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        runner.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (runner.openUpvalues != NULL &&
           runner.openUpvalues->location >= last) {
        PtrPreval* upvalue = runner.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        runner.openUpvalues = upvalue->next;
    }
}

static void defineMethod(PtrString* name) {
    Value method = peek(0);
    PtrQlass* klass = AS_CLASS(peek(1));
    registerSet(&klass->methods, name, method);
    pop();
}

static bool isFalsey(Value value) {
    return (
        IS_NULL(value) || 
        (IS_BOOL(value) && !AS_BOOL(value)) || (
        IS_NUMBER(value) && AS_NUMBER(value) == 0)
   );
}

static void concatenate() {
    PtrString* b = AS_STRING(peek(0));
    PtrString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    PtrString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run() {
    CallFrame* frame = &runner.frames[runner.frameCount - 1];

    #define READ_BYTE() (*frame->ip++)
    #define READ_SHORT() \
        (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    #define READ_CONSTANT() \
        (frame->closure->function->segment.constants.values[READ_BYTE()])

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
                runtimeError("Operands must be numbers."); \
                return SQR_INTRP_ERROR_RUNTIME; \
            } \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while (false)

    #define BITWISE_OP(valueType, op) \
        do { \
            if (!IS_INTEGER(peek(0)) || !IS_INTEGER(peek(1))) { \
                runtimeError("Operands must be numbers."); \
                return SQR_INTRP_ERROR_RUNTIME; \
            } \
            int64 b = (int64)AS_NUMBER(pop()); \
            int64 a = (int64)AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while (false)

    for (;;) {
        #ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(&frame->closure->function->chunk,
                               (int)(frame->ip - frame->closure->function->chunk.code));
        #endif

        byte instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NULL: push(NULL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                byte slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                byte slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_GET_GLOBAL: {
                PtrString* name = READ_STRING();
                Value value;
                if (!registerGet(&runner.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                PtrString* name = READ_STRING();
                registerSet(&runner.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                PtrString* name = READ_STRING();
                if (registerSet(&runner.globals, name, peek(0))) {
                    registerDelete(&runner.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                break;
            }
            case OP_GET_UPVALUE: {
                byte slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                byte slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Only instances have properties.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }

                PtrInstance* instance = AS_INSTANCE(peek(0));
                PtrString* name = READ_STRING();

                Value value;
                if (registerGet(&instance->fields, name, &value)) {
                    pop();
                    push(value);
                    break;
                }
                if (!bindMethod(instance->klass, name)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                break;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Only instances have fields.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }

                PtrInstance* instance = AS_INSTANCE(peek(1));
                registerSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }
            case OP_GET_SUPER: {
                PtrString* name = READ_STRING();
                PtrQlass* superclass = AS_CLASS(pop());

                if (!bindMethod(superclass, name)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, > ); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, < ); break;
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError(
                        "Operands must be two numbers or two strings.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                break;
            }
            case OP_SUBTRACT:       BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;
            case OP_BITWISE_AND:    BITWISE_OP(NUMBER_VAL, &); break;
            case OP_BITWISE_OR:     BITWISE_OP(NUMBER_VAL, |); break;
            case OP_BITWISE_XOR:    BITWISE_OP(NUMBER_VAL, ^); break;
            case OP_BITWISE_NOT:
                if (!IS_INTEGER(peek(0))) {
                    runtimeError("operand must be an integer.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                push(NUMBER_VAL(~(int64)AS_NUMBER(pop())));
                break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_PRINT: {
                printf(" :> ");
                //consoleWriteLine(valueToString(pop()));
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_TYPEOF: {
                printf(" :> ");
                printType(pop());
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                frame = &runner.frames[runner.frameCount - 1];
                break;
            }
            case OP_INVOKE: {
                PtrString* method = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(method, argCount)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                frame = &runner.frames[runner.frameCount - 1];
                break;
            }
            case OP_SUPER_INVOKE: {
                PtrString* method = READ_STRING();
                int argCount = READ_BYTE();
                PtrQlass* superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                frame = &runner.frames[runner.frameCount - 1];
                break;
            }
            case OP_CLOSURE: {
                PtrFunq* function = AS_FUNCTION(READ_CONSTANT());
                PtrQlosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    byte isLocal = READ_BYTE();
                    byte index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] =
                            captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(runner.stackTop - 1);
                pop();
                break;
            case OP_RETURN: {
                Value result = pop();
                closeUpvalues(frame->slots);
                runner.frameCount--;
                if (runner.frameCount == 0) {
                    pop();
                    return SQR_INTRP_OK;
                }

                runner.stackTop = frame->slots;
                push(result);
                frame = &runner.frames[runner.frameCount - 1];
                break;
            }
            case OP_CLASS:
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            case OP_INHERIT: {
                Value superclass = peek(1);
                if (!IS_CLASS(superclass)) {
                    runtimeError("Superclass must be a class.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }

                PtrQlass* subclass = AS_CLASS(peek(0));
                registerAddAll(&AS_CLASS(superclass)->methods,
                            &subclass->methods);
                pop();
                break;
            }
            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
        }
    }
    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
}

void hack(bool b) {
    run();
    if (b) hack(false);
}

InterpretResult interpret(const char* source) {
    PtrFunq* function = digest(source);
    if (function == NULL) return SQR_INTRP_ERROR_DIGEST;
    push(OBJ_VAL(function));
    PtrQlosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);
    return run();
}
