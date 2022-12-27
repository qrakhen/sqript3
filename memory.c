#include <stdlib.h>

#include "digester.h"
#include "memory.h"
#include "runner.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define __GC_GROW 2

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    runner.bAlloc += newSize - oldSize;
    if (newSize > oldSize) {
    #ifdef DEBUG_STRESS_GC
        collectGarbage();
    #endif

        if (runner.bAlloc > runner.__gcNext) {
            __gcRunCycle();
        }
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

void __gcTargetPtr(Ptr* object) {
    if (object == NULL) return;
    if (object->__gcFree) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)ptr);
    printValue(PTR_VAL(ptr));
    printf("\n");
#endif

    object->__gcFree = true;

    if (runner.__gcLimit < runner.__gcCount + 1) {
        runner.__gcLimit = NEXT_SIZE(runner.__gcLimit);
        runner.__gcStack = (Ptr**)realloc(runner.__gcStack,
                                      sizeof(Ptr*) * runner.__gcLimit);
        if (runner.__gcStack == NULL) exit(1);
    }

    runner.__gcStack[runner.__gcCount++] = object;
}

void __gcTargetValue(Value value) {
    if (IS_PTR(value)) __gcTargetPtr(AS_PTR(value));
}

static void markArray(ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        __gcTargetValue(array->values[i]);
    }
}

static void blackenObject(Ptr* ptr) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)ptr);
    printValue(PTR_VAL(ptr));
    printf("\n");
#endif

    switch (ptr->type) {
        case PTR_METHOD: {
            PtrMethod* bound = (PtrMethod*)ptr;
            __gcTargetValue(bound->target);
            __gcTargetPtr((Ptr*)bound->method);
            break;
        }
        case PTR_QLASS: {
            PtrQlass* qlass = (PtrQlass*)ptr;
            __gcTargetPtr((Ptr*)qlass->name);
            markRegister(&qlass->methods);
            break;
        }
        case PTR_QLOSURE: {
            PtrQlosure* qlosure = (PtrQlosure*)ptr;
            __gcTargetPtr((Ptr*)qlosure->function);
            for (int i = 0; i < qlosure->revalCount; i++) {
                __gcTargetPtr((Ptr*)qlosure->upvalues[i]);
            }
            break;
        }
        case PTR_FUNQ: {
            PtrFunq* funq = (PtrFunq*)ptr;
            __gcTargetPtr((Ptr*)funq->name);
            markArray(&funq->segment.constants);
            break;
        }
        case PTR_INSTANCE: {
            PtrInstance* instance = (PtrInstance*)ptr;
            __gcTargetPtr((Ptr*)instance->qlass);
            markRegister(&instance->fields);
            break;
        }
        case PTR_PREVAL:
            __gcTargetValue(((PtrPreval*)ptr)->closed);
            break;
        case PTR_NATIVE:
        case PTR_STRING:
            break;
    }
}

static void freeObject(Ptr* object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)ptr, ptr->type);
#endif

    switch (object->type) {
        case PTR_METHOD:
            FREE(PtrMethod, object);
            break;
        case PTR_QLASS: {
            PtrQlass* qlass = (PtrQlass*)object;
            freeRegister(&qlass->methods);
            FREE(PtrQlass, object);
            break;
        } // [braces]
        case PTR_QLOSURE: {
            PtrQlosure* qlosure = (PtrQlosure*)object;
            ARR_FREE(PtrPreval*, qlosure->upvalues,
                       qlosure->revalCount);
            FREE(PtrQlosure, object);
            break;
        }
        case PTR_FUNQ: {
            PtrFunq* funq = (PtrFunq*)object;
            freeSegment(&funq->segment);
            FREE(PtrFunq, object);
            break;
        }
        case PTR_INSTANCE: {
            PtrInstance* instance = (PtrInstance*)object;
            freeRegister(&instance->fields);
            FREE(PtrInstance, object);
            break;
        }
        case PTR_NATIVE:
            FREE(PtrNative, object);
            break;
        case PTR_STRING: {
            PtrString* string = (PtrString*)object;
            ARR_FREE(char, string->chars, string->length + 1);
            FREE(PtrString, object);
            break;
        }
        case PTR_PREVAL:
            FREE(PtrPreval, object);
            break;
    }
}

static void markRoots() {
    for (Value* slot = runner.stack; slot < runner.cursor; slot++) {
        __gcTargetValue(*slot);
    }

    for (int i = 0; i < runner.qc; i++) {
        __gcTargetPtr((Ptr*)runner.qalls[i].qlosure);
    }

    for (PtrPreval* upvalue = runner.__openPrevals;
         upvalue != NULL;
         upvalue = upvalue->next) {
        __gcTargetPtr((Ptr*)upvalue);
    }

    markRegister(&runner.globals);
    markCompilerRoots();
    __gcTargetPtr((Ptr*)runner.__initString);
}

static void traceReferences() {
    while (runner.__gcCount > 0) {
        Ptr* object = runner.__gcStack[--runner.__gcCount];
        blackenObject(object);
    }
}

static void sweep() {
    Ptr* previous = NULL;
    Ptr* object = runner.pointers;
    while (object != NULL) {
        if (object->__gcFree) {
            object->__gcFree = false;
            previous = object;
            object = object->next;
        }
        else {
            Ptr* unreached = object;
            object = object->next;
            if (previous != NULL) {
                previous->next = object;
            }
            else {
                runner.pointers = object;
            }

            freeObject(unreached);
        }
    }
}

void __gcRunCycle() {
    #ifdef DEBUG_LOG_GC
        printf("-- gc begin\n");
        size_t before = vm.bAlloc;
    #endif

    markRoots();
    traceReferences();
    registerRemoveWhite(&runner.strings);
    sweep();

    runner.__gcNext = runner.bAlloc * __GC_GROW;

    #ifdef DEBUG_LOG_GC
        printf("-- gc end\n");
        printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
               before - runner.bAlloc, before, runner.bAlloc, runner.__gcNext);
    #endif
}

void __freePointers() {
    Ptr* object = runner.pointers;
    while (object != NULL) {
        Ptr* next = object->next;
        freeObject(object);
        object = next;
    }

    free(runner.__gcStack);
}
