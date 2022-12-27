#include <stdlib.h>

#include "digester.h"
#include "memory.h"
#include "runner.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    runner.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize) {
    #ifdef DEBUG_STRESS_GC
        collectGarbage();
    #endif

        if (runner.bytesAllocated > runner.__gcTrigger) {
            collectGarbage();
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

void markObject(Ptr* object) {
    if (object == NULL) return;
    if (object->__gcFree) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    object->__gcFree = true;

    if (runner.__gcLimit < runner.__gcCount + 1) {
        runner.__gcLimit = GROW_CAPACITY(runner.__gcLimit);
        runner.__gcStack = (Ptr**)realloc(runner.__gcStack,
                                      sizeof(Ptr*) * runner.__gcLimit);
        if (runner.__gcStack == NULL) exit(1);
    }

    runner.__gcStack[runner.__gcCount++] = object;
}

void markValue(Value value) {
    if (IS_PTR(value)) markObject(AS_OBJ(value));
}

static void markArray(ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        markValue(array->values[i]);
    }
}

static void blackenObject(Ptr* object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case PTR_METHOD: {
            PtrMethod* bound = (PtrMethod*)object;
            markValue(bound->receiver);
            markObject((Ptr*)bound->method);
            break;
        }
        case PTR_QLASS: {
            PtrQlass* klass = (PtrQlass*)object;
            markObject((Ptr*)klass->name);
            markRegister(&klass->methods);
            break;
        }
        case PTR_QLOSURE: {
            PtrQlosure* closure = (PtrQlosure*)object;
            markObject((Ptr*)closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject((Ptr*)closure->upvalues[i]);
            }
            break;
        }
        case PTR_FUNQ: {
            PtrFunq* function = (PtrFunq*)object;
            markObject((Ptr*)function->name);
            markArray(&function->segment.constants);
            break;
        }
        case PTR_INSTANCE: {
            PtrInstance* instance = (PtrInstance*)object;
            markObject((Ptr*)instance->klass);
            markRegister(&instance->fields);
            break;
        }
        case PTR_PREVAL:
            markValue(((PtrPreval*)object)->closed);
            break;
        case PTR_NATIVE:
        case PTR_STRING:
            break;
    }
}

static void freeObject(Ptr* object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)object, object->type);
#endif

    switch (object->type) {
        case PTR_METHOD:
            FREE(PtrMethod, object);
            break;
        case PTR_QLASS: {
            PtrQlass* klass = (PtrQlass*)object;
            freeRegister(&klass->methods);
            FREE(PtrQlass, object);
            break;
        } // [braces]
        case PTR_QLOSURE: {
            PtrQlosure* closure = (PtrQlosure*)object;
            FREE_ARRAY(PtrPreval*, closure->upvalues,
                       closure->upvalueCount);
            FREE(PtrQlosure, object);
            break;
        }
        case PTR_FUNQ: {
            PtrFunq* function = (PtrFunq*)object;
            freeSegment(&function->segment);
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
            FREE_ARRAY(char, string->chars, string->length + 1);
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
        markValue(*slot);
    }

    for (int i = 0; i < runner.contextCount; i++) {
        markObject((Ptr*)runner.contextStack[i].closure);
    }

    for (PtrPreval* upvalue = runner.openUpvalues;
         upvalue != NULL;
         upvalue = upvalue->next) {
        markObject((Ptr*)upvalue);
    }

    markRegister(&runner.globals);
    markCompilerRoots();
    markObject((Ptr*)runner.initString);
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

void collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = vm.bytesAllocated;
#endif

    markRoots();
    traceReferences();
    registerRemoveWhite(&runner.strings);
    sweep();

    runner.__gcTrigger = runner.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated,
           vm.nextGC);
#endif
}

void freeObjects() {
    Ptr* object = runner.pointers;
    while (object != NULL) {
        Ptr* next = object->next;
        freeObject(object);
        object = next;
    }

    free(runner.__gcStack);
}
