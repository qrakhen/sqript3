#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "register.h"
#include "value.h"
#include "runner.h"
#include "types.h"

#define ALLOCATE_PTR(type, objectType) \
    (type*)allocatePtr(sizeof(type), objectType)

Ptr* allocatePtr(size_t size, PtrType type) {
    Ptr* object = (Ptr*)reallocate(NULL, 0, size);
    object->type = type;
    object->__gcFree = false;

    object->next = runner.pointers;
    runner.pointers = object;

    #ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
    #endif

    return object;
}

PtrTargetedNativeMethod* newTargetedNativeMethod(Value target, PtrNativeMethod* method) {
    PtrTargetedNativeMethod* fn = ALLOCATE_PTR(PtrTargetedNativeMethod, PTR_METHOD);
    fn->target = target;
    fn->method = method->method;
    return fn;
}

PtrNativeMethod* newNativeMethod(Value target, NativeMethod method) {
    PtrNativeMethod* fn = ALLOCATE_PTR(PtrNativeMethod, PTR_METHOD);
    fn->method = method;
    return fn;
}

PtrMethod* newBoundMethod(Value target, PtrQlosure* member) {
    PtrMethod* bound = ALLOCATE_PTR(PtrMethod, PTR_METHOD);
    bound->target = target;
    bound->method = member;
    return bound;
}

PtrQlass* newClass(PtrString* name) {
    PtrQlass* qlass = ALLOCATE_PTR(PtrQlass, PTR_QLASS);
    qlass->name = name;
    initRegister(&qlass->methods);
    initRegister(&qlass->properties);
    return qlass;
}

PtrQlosure* newClosure(PtrFunq* function) {
    PtrPreval** revals = ALLOC(PtrPreval*,
                                     function->revalCount);
    for (int i = 0; i < function->revalCount; i++) {
        revals[i] = NULL;
    }

    PtrQlosure* qlosure = ALLOCATE_PTR(PtrQlosure, PTR_QLOSURE);
    qlosure->function = function;
    qlosure->upvalues = revals;
    qlosure->revalCount = function->revalCount;
    return qlosure;
}

PtrFunq* newFunction() {
    PtrFunq* function = ALLOCATE_PTR(PtrFunq, PTR_FUNQ);
    function->argc = 0;
    function->revalCount = 0;
    function->name = NULL;
    initSegment(&function->segment);
    return function;
}

PtrInstance* newInstance(PtrQlass* qlass) {
    PtrInstance* instance = ALLOCATE_PTR(PtrInstance, PTR_INSTANCE);
    instance->qlass = qlass;
    initRegister(&instance->fields);
    return instance;
}

PtrNative* newNative(NativeFunq function) {
    PtrNative* native = ALLOCATE_PTR(PtrNative, PTR_NATIVE);
    native->function = function;
    return native;
}

PtrPreval* newUpvalue(Value* slot) {
    PtrPreval* upvalue = ALLOCATE_PTR(PtrPreval, PTR_PREVAL);
    upvalue->closed = NULL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

static void printFunction(PtrFunq* function) {
    if (function->name == NULL) {
        printf("<sqript>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void printObject(Value value) {
    switch (PTR_TYPE(value)) {
        case PTR_METHOD:
            printFunction(AS_BOUND_METHOD(value)->method->function);
            break;
        case PTR_QLASS:
            printf("%s", AS_CLASS(value)->name->chars);
            break;
        case PTR_QLOSURE:
            printFunction(AS_CLOSURE(value)->function);
            break;
        case PTR_FUNQ:
            printFunction(AS_FUNCTION(value));
            break;
        case PTR_INSTANCE:
            printf("%s instance",
                   AS_INSTANCE(value)->qlass->name->chars);
            break;        
        case PTR_NATIVE:
            printf("<native fn>");
            break;
        case PTR_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case PTR_PREVAL:
            printf("upvalue");
            break;
    }
}