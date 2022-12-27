#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "register.h"
#include "value.h"
#include "runner.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocatePtr(sizeof(type), objectType)

Ptr* allocatePtr(size_t size, PtrType type) {
    Ptr* object = (Ptr*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = runner.objects;
    runner.objects = object;

    #ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
    #endif

    return object;
}

PtrMethod* newBoundMethod(Value receiver,
                               PtrQlosure* method) {
    PtrMethod* bound = ALLOCATE_OBJ(PtrMethod,
                                         PTR_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

PtrQlass* newClass(PtrString* name) {
    PtrQlass* klass = ALLOCATE_OBJ(PtrQlass, PTR_QLASS);
    klass->name = name; // [klass]
    initRegister(&klass->methods);
    return klass;
}

PtrQlosure* newClosure(PtrFunq* function) {
    PtrPreval** upvalues = ALLOCATE(PtrPreval*,
                                     function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    PtrQlosure* closure = ALLOCATE_OBJ(PtrQlosure, PTR_QLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

PtrFunq* newFunction() {
    PtrFunq* function = ALLOCATE_OBJ(PtrFunq, PTR_FUNQ);
    function->argc = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initSegment(&function->segment);
    return function;
}

PtrInstance* newInstance(PtrQlass* klass) {
    PtrInstance* instance = ALLOCATE_OBJ(PtrInstance, PTR_INSTANCE);
    instance->klass = klass;
    initRegister(&instance->fields);
    return instance;
}

PtrNative* newNative(NativeFn function) {
    PtrNative* native = ALLOCATE_OBJ(PtrNative, PTR_NATIVE);
    native->function = function;
    return native;
}

static PtrString* allocateString(char* chars, int length,
                                 uint32_t hash) {
    PtrString* string = ALLOCATE_OBJ(PtrString, PTR_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    registerSet(&runner.strings, string, NULL_VAL);
    pop();

    return string;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (Byte)key[i];
        hash *= 16777619;
    }
    return hash;
}

PtrString* takeString(char* chars, int length) {

    uint32_t hash = hashString(chars, length);
    PtrString* interned = registerFindString(&runner.strings, chars, length,
                                             hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

PtrString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    PtrString* interned = registerFindString(&runner.strings, chars, length,
                                             hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

PtrPreval* newUpvalue(Value* slot) {
    PtrPreval* upvalue = ALLOCATE_OBJ(PtrPreval, PTR_PREVAL);
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
    switch (OBJ_TYPE(value)) {
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
                   AS_INSTANCE(value)->klass->name->chars);
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
