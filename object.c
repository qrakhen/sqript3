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

    #if DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
    #endif

    return object;
}

PtrTargetedNativeMethod* newTargetedNativeMethod(Value target, PtrNativeMethod* method) {
    PtrTargetedNativeMethod* fn = ALLOCATE_PTR(PtrTargetedNativeMethod, PTR_NATIVE_METHOD);
    fn->target = target;
    fn->method = method;
    return fn;
}

PtrNativeMethod* newNativeMethod(NativeMethod callback) {
    PtrNativeMethod* fn = ALLOCATE_PTR(PtrNativeMethod, PTR_NATIVE_METHOD);
    fn->callback = callback;
    return fn;
}

Method* newBoundMethod(Value target, Qontext* member) {
    Method* bound = ALLOCATE_PTR(Method, PTR_METHOD);
    bound->target = target;
    bound->method = member;
    return bound;
}

Qlass* newClass(String* name) {
    Qlass* qlass = ALLOCATE_PTR(Qlass, PTR_QLASS);
    qlass->name = name;
    initRegister(&qlass->methods);
    initRegister(&qlass->properties);
    return qlass;
}

Qontext* newClosure(Funqtion* function) {
    PtrPreval** revals = ALLOC(PtrPreval*, function->revalCount);
    for (int i = 0; i < function->revalCount; i++) {
        revals[i] = NULL;
    }

    Qontext* qlosure = ALLOCATE_PTR(Qontext, PTR_QLOSURE);
    qlosure->function = function;
    qlosure->upvalues = revals;
    qlosure->revalCount = function->revalCount;
    return qlosure;
}

Funqtion* newFunction() {
    Funqtion* function = ALLOCATE_PTR(Funqtion, PTR_FUNQ);
    function->argc = 0;
    function->revalCount = 0;
    function->name = NULL;
    initSegment(&function->segment);
    return function;
}

Objeqt* newInstance(Qlass* qlass) {
    Objeqt* instance = ALLOCATE_PTR(Objeqt, PTR_OBJEQT);
    instance->qlass = qlass;
    initRegister(&instance->fields);
    return instance;
}

Module* newModule(String* name, String* source) {
    Module* _m = ALLOCATE_PTR(Module, PTR_MODULE);
    _m->name = name;
    _m->source = source;
    initRegister(&_m->fields);
    initRegister(&_m->exports);
    return _m;
}

NativeQall* newNative(NativeFunq function) {
    NativeQall* native = ALLOCATE_PTR(NativeQall, PTR_NATIVE_FUNQ);
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

static void printFunction(Funqtion* function) {
    if (function->name == NULL) {
        printf("<sqript>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void printObject(Value value) {
    switch (PTR_TYPE(value)) {
        case PTR_METHOD:
            printFunction(AS_METHOD(value)->method->function);
            break;
        case PTR_QLASS:
            printf("%s", AS_QLASS(value)->name->chars);
            break;
        case PTR_QLOSURE:
            printFunction(AS_QONTEXT(value)->function);
            break;
        case PTR_FUNQ:
            printFunction(AS_FUNQ(value));
            break;
        case PTR_OBJEQT:
            printf("%s instance",
                   AS_OBJEQT(value)->qlass->name->chars);
            break;        
        case PTR_NATIVE_FUNQ:
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