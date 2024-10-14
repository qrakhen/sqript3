#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "common.h"
#include "digester.h"
#include "debug.h"
#include "object.h"
#include "array.h"
#include "memory.h"
#include "runner.h"
#include "console.h"
#include "native.h"
#include "string.h"
#include "io.h"

Runner runner;

typedef int (*OperationCallback)();

typedef struct {
    OperationCallback fn;
} Operation;

static void resetStack() {
    runner.cursor = runner.stack;
    runner.qc = 0;
    runner.__openPrevals = NULL;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    consoleSetColor(C_COLOR_RED);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = runner.qc - 1; i >= 0; i--) {
        Qall* frame = &runner.qalls[i];
        Funqtion* function = frame->qlosure->function;
        size_t instruction = frame->ip - function->segment.code - 1;
        fprintf(stderr, "[line %d] in ",
                function->segment.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    consoleResetColor();
    resetStack();
}

void defineNative(const char* name, NativeFunq function) {
    push(PTR_VAL(makeString(name, (int)strlen(name))));
    push(PTR_VAL(newNative(function)));
    registerSet(&runner.globals, AS_STRING(runner.stack[0]), runner.stack[1]);
    pop();
    pop();
}

void initRunner(bool initial) {
    resetStack();
    runner.pointers = NULL;
    runner.bAlloc = 0;

    initRegister(&runner.globals);
    initRegister(&runner.imports);

    if (initial) {
        initRegister(&runner.__cachedImports);
        initRegister(&runner.exports);
        initRegister(&runner.strings);
    } else {
        if (&runner.exports.entries != NULL) {
            for (int i = 0; i < runner.exports.count; i++) {
                if (runner.exports.entries[i].key == NULL)
                    continue;
                printf("%s > ", runner.exports.entries[i].key->chars);
                printValue(runner.exports.entries[i].value);
                printf("\n");
            }             
        }
    }

    runner.__gcNext = 1024 * 1024;
    runner.__gcCount = 0;
    runner.__gcLimit = 0;
    runner.__gcStack = NULL;

    runner.__initString = NULL;
    runner.__initString = makeString("init", 4);

    initNativeMethods();
}

void freeRunner() {
    freeRegister(&runner.globals);
    freeRegister(&runner.strings);
    freeRegister(&runner.imports);
    runner.__initString = NULL;
    __freePointers();
}

void push(Value value) { *(runner.cursor++) = value; }
Value pop() { return *(--runner.cursor); }
static Value peek(int distance) { return runner.cursor[-1 - distance]; }

static bool call(Qontext* qlosure, int argCount) {
    if (argCount != qlosure->function->argc) {
        runtimeError("Expected %d arguments but got %d.", qlosure->function->argc, argCount);
        return false;
    }

    if (runner.qc == MAX_QALLS) {
        runtimeError("Stack overflow.");
        return false;
    }

    Qall* frame = &runner.qalls[runner.qc++];
    frame->qlosure = qlosure;
    frame->ip = qlosure->function->segment.code;
    frame->slots = runner.cursor - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_PTR(callee)) {
        switch (PTR_TYPE(callee)) {
            case PTR_NATIVE_METHOD: {
                PtrTargetedNativeMethod* targeted = AS_TNMETHOD(callee);
                NativeMethod fn = targeted->method->callback;
                Value result = fn(targeted->target, argCount, runner.cursor - (argCount + 1));
                runner.cursor -= argCount + 2;
                push(result);
                return true;
            }
            case PTR_METHOD: {
                Method* bound = AS_METHOD(callee);
                runner.cursor[-argCount - 1] = bound->target;
                return call(bound->method, argCount);
            }
            case PTR_QLASS: {
                Qlass* qlass = AS_QLASS(callee);
                runner.cursor[-argCount - 1] = PTR_VAL(newInstance(qlass));
                Value initializer;
                if (registerGet(&qlass->methods, runner.__initString, &initializer)) {
                    return call(AS_QONTEXT(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }
                return true;
            }
            case PTR_QLOSURE:
                return call(AS_QONTEXT(callee), argCount);
            case PTR_NATIVE_FUNQ: {
                NativeFunq native = AS_NATIVE(callee);
                Value result = native(argCount, runner.cursor - argCount);
                runner.cursor -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break;
        }
    }
    printType(callee);
    runtimeError("can not call value of that type");
    return false;
}

static bool invokeFromClass(Qlass* qlass, String* name, int argCount) {
    Value member;
    if (!registerGet(&qlass->methods, name, &member)) {
        runtimeError("unknown member '%s'.", name->chars);
        return false;
    }
    return call(AS_QONTEXT(member), argCount);
}

static bool invoke(String* name, int argCount) {
    Value target = peek(argCount);
    if (!IS_OBJEQT(target)) {
        PtrTargetedNativeMethod* targeted = bindNativeMethod(target, name);
        if (targeted != NULL) {
            if (targeted->method->minArgs > argCount) {
                runtimeError("%s expects at least %d arguments but got %d", name->chars, targeted->method->minArgs, argCount);
                return false;
            }
            if (targeted->method->maxArgs < argCount) {
                runtimeError("%s expects at most %d arguments but got %d", name->chars, targeted->method->maxArgs, argCount);
                return false;
            }
            push(PTR_VAL(targeted));
            return callValue(PTR_VAL(targeted), argCount);
        } 
        runtimeError("could not find method %s", name->chars);
        return false;
    } else {
        Objeqt* instance = AS_OBJEQT(target);

        Value value;
        if (registerGet(&instance->fields, name, &value)) {
            runner.cursor[-argCount - 1] = value;
            return callValue(value, argCount);
        }

        return invokeFromClass(instance->qlass, name, argCount);
    }
}

static bool bindMethod(Qlass* qlass, String* name) {
    Value member;
    if (!registerGet(&qlass->methods, name, &member)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    Method* bound = newBoundMethod(peek(0), AS_QONTEXT(member));
    pop();
    push(PTR_VAL(bound));
    return true;
}

static PtrPreval* captureUpvalue(Value* local) {
    PtrPreval* prevUpvalue = NULL;
    PtrPreval* upvalue = runner.__openPrevals;
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
        runner.__openPrevals = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (runner.__openPrevals != NULL &&
           runner.__openPrevals->location >= last) {
        PtrPreval* upvalue = runner.__openPrevals;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        runner.__openPrevals = upvalue->next;
    }
}

static void defineMethod(String* name) {
    Value member = peek(0);
    Qlass* qlass = AS_QLASS(peek(1));
    registerSet(&qlass->methods, name, member);
    pop();
}

static void defineProperty(String* name) {
    Value member = peek(0);
    Qlass* qlass = AS_QLASS(peek(1));
    registerSet(&qlass->properties, name, member);
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
    String* b = AS_STRING(peek(0));
    String* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOC(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    String* result = takeString(chars, length);
    pop();
    pop();
    push(PTR_VAL(result));
}

static InterpretResult run() {
    Qall* frame = &runner.qalls[runner.qc - 1];

    #define READ_BYTE() (*frame->ip++)
    #define READ_SHORT() \
        (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    #define READ_CONSTANT() \
        (frame->qlosure->function->segment.constants.values[READ_BYTE()])

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

    #define CREMENT_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0))) { \
                runtimeError("pperand must be a number"); \
                return SQR_INTRP_ERROR_RUNTIME; \
            } \
            double a = AS_NUMBER(pop()); \
            push(valueType(a op)); \
        } while (false)

    #define BITWISE_OP(valueType, op) \
        do { \
            if (!MAYBE_INT(peek(0)) || !MAYBE_INT(peek(1))) { \
                runtimeError("Operands must be integers."); \
                return SQR_INTRP_ERROR_RUNTIME; \
            } \
            Int b = (Int)AS_NUMBER(pop()); \
            Int a = (Int)AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while (false)

    for (;;) {
        #if __DBG_TRACE        
        printf("          ");
        for (Value* slot = runner.stack; slot < runner.cursor; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        __dbgDissectOp(&frame->qlosure->function->segment,
                               (int)(frame->ip - frame->qlosure->function->segment.code));
        #endif

        Byte instruction;
        //if (frame->ip == NULL)
            //return SQR_INTRP_OK;
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
                Byte slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                Byte slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_GET_GLOBAL: {
                String* name = READ_STRING();
                Value value;
                if (!registerGet(&runner.globals, name, &value)) {
                    if (!registerGet(&runner.exports, name, &value)) { // @TODO TEMP
                        runtimeError("Undefined variable '%s'.", name->chars);
                        return SQR_INTRP_ERROR_RUNTIME;
                    }
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                String* name = READ_STRING();
                registerSet(&runner.globals, name, peek(0));
                registerSet(&runner.exports, name, peek(0)); // @TODO TEMP
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                String* name = READ_STRING();
                if (registerSet(&runner.globals, name, peek(0))) {
                    registerDelete(&runner.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                break;
            }
            case OP_GET_UPVALUE: {
                Byte slot = READ_BYTE();
                push(*frame->qlosure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                Byte slot = READ_BYTE();
                *frame->qlosure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_GET_PROPERTY: {
                // native methods
                if (!IS_OBJEQT(peek(0))) {
                    String* name = READ_STRING();
                    Value value = peek(0);
                    PtrTargetedNativeMethod* method = bindNativeMethod(value, name);
                    if (method != NULL) {
                        pop();
                        pop();
                        push(PTR_VAL(method));
                        break;
                    }
                    runtimeError("only instances have properties.");
                    return SQR_INTRP_ERROR_RUNTIME;
                } else {
                    String* name = READ_STRING();
                    Objeqt* instance = AS_OBJEQT(peek(0));

                    Value value;
                    if (registerGet(&instance->fields, name, &value)) {
                        pop();
                        push(value);
                        break;
                    }
                    if (!bindMethod(instance->qlass, name)) {
                        return SQR_INTRP_ERROR_RUNTIME;
                    }
                    break;
                }
            }
            case OP_SET_PROPERTY: {
                if (!IS_OBJEQT(peek(1))) {
                    runtimeError("Only instances have fields.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }

                Objeqt* instance = AS_OBJEQT(peek(1));
                registerSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }
            case OP_GET_SUPER: {
                String* name = READ_STRING();
                Qlass* superclass = AS_QLASS(pop());

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
            case OP_INCREMENT:      CREMENT_OP(NUMBER_VAL, ++ ); break;
            case OP_DECREMENT:      CREMENT_OP(NUMBER_VAL, -- ); break;
            case OP_BITWISE_AND:    BITWISE_OP(NUMBER_VAL, &); break;
            case OP_BITWISE_OR:     BITWISE_OP(NUMBER_VAL, |); break;
            case OP_BITWISE_XOR:    BITWISE_OP(NUMBER_VAL, ^); break;
            case OP_BITWISE_NOT:
                if (!MAYBE_INT(peek(0))) {
                    runtimeError("operand must be an integer.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                push(NUMBER_VAL(~(Int)AS_NUMBER(pop())));
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
            case OP_REF:
                Value r = pop();
                push(REF_VAL(r));
                break;
            case OP_PRINT: {
                consoleSetColor(C_COLOR_GREEN);
                printf(" :> ");
                //consoleWriteLine(valueToString(pop()));
                printValue(pop());
                printf("\n");
                consoleResetColor();
                break;
            }
            case OP_PRINT_EXPR: {
                consoleSetColor(C_COLOR_DGRAY);
                printf(" ~: ");
                ///consoleWriteLine(valueToString(peek(-1)));
                printValue(peek(0));
                printf("\n");
                consoleResetColor();
                break;
            }
            case OP_TYPEOF: {
                printf(" :> ");
                COUTLNC(printType(pop()), C_COLOR_YELLOW);
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
                frame = &runner.qalls[runner.qc - 1];
                break;
            }
            case OP_INVOKE: {
                String* member = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(member, argCount)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                frame = &runner.qalls[runner.qc - 1];
                break;
            }
            case OP_SUPER_INVOKE: {
                String* member = READ_STRING();
                int argCount = READ_BYTE();
                Qlass* superclass = AS_QLASS(pop());
                if (!invokeFromClass(superclass, member, argCount)) {
                    return SQR_INTRP_ERROR_RUNTIME;
                }
                frame = &runner.qalls[runner.qc - 1];
                break;
            }
            case OP_ARRAY: {
                int length = AS_NUMBER(READ_CONSTANT());
                PtrArray* arr = createArray(length, T_ANY);
                for (int i = length - 1; i >= 0; i--) {
                    arr->values[i] = pop();
                }
                push(PTR_VAL(arr));
                break;
            }
            case OP_ARRAY_GET: {
                int index = AS_NUMBER(pop());
                PtrArray* arr = AS_ARRAY(pop());
                push(arr->values[index]);
                break;
            }
            case OP_ARRAY_SET: {
                Value value = pop();
                int index = AS_NUMBER(pop());
                PtrArray* arr = AS_ARRAY(peek(0));
                arr->values[index] = value;
                break;
            }
            case OP_ARRAY_ADD: {
                PtrArray* arr = AS_ARRAY(pop());
                Value value = pop();
                arrayAppend(arr, value);
                break;
            }
            case OP_ARRAY_REMOVE: {
                pop(); pop();
                break;
            }
            case OP_CLOSURE: {
                Funqtion* function = AS_FUNQ(READ_CONSTANT());
                Qontext* qlosure = newClosure(function);
                push(PTR_VAL(qlosure));
                for (int i = 0; i < qlosure->revalCount; i++) {
                    Byte isLocal = READ_BYTE();
                    Byte index = READ_BYTE();
                    if (isLocal) {
                        qlosure->upvalues[i] =
                            captureUpvalue(frame->slots + index);
                    } else {
                        qlosure->upvalues[i] = frame->qlosure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(runner.cursor - 1);
                pop();
                break;
            case OP_RETURN: {
                Value result = pop();
                closeUpvalues(frame->slots);
                runner.qc--;
                if (runner.qc == 0) {
                    pop();
                    return SQR_INTRP_OK;
                }

                runner.cursor = frame->slots;
                push(result);
                frame = &runner.qalls[runner.qc - 1];
                break;
            }
            case OP_CLASS:
                push(PTR_VAL(newClass(READ_STRING())));
                break;
            case OP_INHERIT: {
                Value superclass = peek(1);
                if (!IS_QLASS(superclass)) {
                    runtimeError("Superclass must be a class.");
                    return SQR_INTRP_ERROR_RUNTIME;
                }

                Qlass* subclass = AS_QLASS(peek(0));
                registerAddAll(&AS_QLASS(superclass)->methods,
                            &subclass->methods);
                pop();
                break;
                break;
            }
            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
            case OP_PROPERTY:
                defineProperty(READ_STRING());
                break;
            case OP_EXPORT:
                String* name = AS_STRING(pop());
                registerSet(&runner.exports, name, NUMBER_VAL(1));
                pop();
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

InterpretResult interpret(const char* module, const char* source) {
    #if __DBG_PRINT_EXEC_TIME
        double ti = NOW_MS;
    #endif

    printf(" === CURRENT MODULE: %s === \n", module);

    int offset = doImports(source);

    if (module != "__main") {
        Value value;
        if (registerGet(&runner.__cachedImports, makeString(module, strlen(module)), &value)) {
            runtimeError("file %s already read!", module);
            return SQR_INTRP_ERROR_RUNTIME;
        } else {
            registerSet(&runner.__cachedImports, makeString(module, strlen(module)), NUMBER_VAL(1));
        }
    }

    const char* _source = source + offset;

    Funqtion* function = digest(_source);
    if (function == NULL) return SQR_INTRP_ERROR_DIGEST;
    __dbgDissect(&function->segment, "everything");

    push(PTR_VAL(function));

    Qontext* qlosure = newClosure(function);
    pop();
    push(PTR_VAL(qlosure));
    call(qlosure, 0);

    #if __DBG_PRINT_EXEC_TIME
        double tc = NOW_MS;
    #endif

    int error = run();

    #if __DBG_PRINT_EXEC_TIME
        double te = NOW_MS;
        double t1 = tc - ti;
        double t2 = te - tc;
        
        COUTLNC(F(
                " :~ Tx: %s | Td: %s", 
                formatTime(t2, TIME_UNIT_MS), 
                formatTime(t1, TIME_UNIT_MS)),
            C_COLOR_DGRAY);
    #endif
    return error;
}
