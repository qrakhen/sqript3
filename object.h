#ifndef sqript_object_h
#define sqript_object_h

#include "common.h"
#include "segment.h"
#include "register.h"
#include "value.h"

#define ALLOCATE_PTR(type, objectType) \
    (type*)allocatePtr(sizeof(type), objectType)

#define PTR_TYPE(value)        (AS_PTR(value)->type)

#define IS_BOUND_METHOD(value) matchPtrType(value, PTR_METHOD)
#define IS_CLASS(value)        matchPtrType(value, PTR_QLASS)
#define IS_CLOSURE(value)      matchPtrType(value, PTR_QLOSURE)
#define IS_FUNCTION(value)     matchPtrType(value, PTR_FUNQ)
#define IS_INSTANCE(value)     matchPtrType(value, PTR_INSTANCE)
#define IS_NATIVE(value)       matchPtrType(value, PTR_NATIVE)

#define AS_BOUND_METHOD(value) ((PtrMethod*)AS_PTR(value))
#define AS_CLASS(value)        ((PtrQlass*)AS_PTR(value))
#define AS_CLOSURE(value)      ((PtrQlosure*)AS_PTR(value))
#define AS_FUNCTION(value)     ((PtrFunq*)AS_PTR(value))
#define AS_INSTANCE(value)     ((PtrInstance*)AS_PTR(value))
#define AS_NATIVE(value)       (((PtrNative*)AS_PTR(value))->function)

typedef enum {
    PTR_METHOD = T_PTR | 1,
    PTR_QLASS = T_PTR | 2,
    PTR_QLOSURE = T_PTR | 4,
    PTR_FUNQ = T_PTR | 8,
    PTR_INSTANCE = T_PTR | 16,
    PTR_ARRAY = T_PTR | 32,
    PTR_LIST = T_PTR | 64,
    PTR_NATIVE = T_PTR | 128,
    PTR_STRING = T_PTR | 256,
    PTR_PREVAL = T_PTR | 512
} PtrType;

struct Ptr {
    PtrType type;
    bool __gcFree;
    struct Ptr* next;
};

typedef struct {
    Ptr ptr;
    Byte argc;
    int revalCount;
    Segment segment;
    String* name;
} PtrFunq;

typedef struct {
    Ptr ptr;
    NativeFunq function;
} PtrNative;

typedef struct PtrPreval {
    Ptr ptr;
    Value* location;
    Value closed;
    struct PtrPreval* next;
} PtrPreval;

typedef struct {
    Ptr ptr;
    PtrFunq* function;
    PtrPreval** upvalues;
    int revalCount;
} PtrQlosure;

typedef struct {
    Ptr ptr;
    String* name;
    Register methods;
    Register properties;
} PtrQlass;

typedef struct {
    Ptr ptr;
    PtrQlass* qlass;
    Register fields;
} PtrInstance;

typedef struct {
    Ptr ptr;
    Value target;
    PtrQlosure* method;
} PtrMethod;

typedef struct {
    Ptr ptr;
    NativeMethod method;
} PtrNativeMethod;

typedef struct {
    Ptr ptr;
    Value target;
    NativeMethod* method;
} PtrTargetedNativeMethod;

Ptr* allocatePtr(size_t size, PtrType type);
PtrQlosure* newClosure(PtrFunq* function);
PtrFunq* newFunction();
PtrQlass* newClass(String* name);
PtrInstance* newInstance(PtrQlass* qlass);
PtrMethod* newBoundMethod(Value target, PtrQlosure* member);

PtrNative* newNative(NativeFunq function);
PtrNativeMethod* newNativeMethod(NativeMethod member);
PtrTargetedNativeMethod* newTargetedNativeMethod(Value target, PtrNativeMethod* member);

PtrPreval* newUpvalue(Value* slot);
void printObject(Value value);

static inline bool matchPtrType(Value value, PtrType type) {
    return IS_PTR(value) && AS_PTR(value)->type == type;
}

#endif
