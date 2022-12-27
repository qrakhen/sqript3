#ifndef sqript_object_h
#define sqript_object_h

#include "common.h"
#include "segment.h"
#include "register.h"
#include "value.h"

#define PTR_TYPE(value)        (AS_PTR(value)->type)

#define IS_BOUND_METHOD(value) matchPtrType(value, PTR_METHOD)
#define IS_CLASS(value)        matchPtrType(value, PTR_QLASS)
#define IS_CLOSURE(value)      matchPtrType(value, PTR_QLOSURE)
#define IS_FUNCTION(value)     matchPtrType(value, PTR_FUNQ)
#define IS_INSTANCE(value)     matchPtrType(value, PTR_INSTANCE)
#define IS_NATIVE(value)       matchPtrType(value, PTR_NATIVE)
#define IS_STRING(value)       matchPtrType(value, PTR_STRING)

#define AS_BOUND_METHOD(value) ((PtrMethod*)AS_PTR(value))
#define AS_CLASS(value)        ((PtrQlass*)AS_PTR(value))
#define AS_CLOSURE(value)      ((PtrQlosure*)AS_PTR(value))
#define AS_FUNCTION(value)     ((PtrFunq*)AS_PTR(value))
#define AS_INSTANCE(value)     ((PtrInstance*)AS_PTR(value))
#define AS_NATIVE(value)       (((PtrNative*)AS_PTR(value))->function)
#define AS_STRING(value)       ((PtrString*)AS_PTR(value))
#define AS_CSTRING(value)      (((PtrString*)AS_PTR(value))->chars)

typedef enum {
    PTR_METHOD,
    PTR_QLASS,
    PTR_QLOSURE,
    PTR_FUNQ,
    PTR_INSTANCE,
    PTR_ARRAY,
    PTR_NATIVE,
    PTR_STRING,
    PTR_PREVAL
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
    PtrString* name;
} PtrFunq;

typedef Value(*NativeFunq)(int argCount, Value* args);
typedef Value(*NativeMethod)(Value target, int argCount, Value* args);

typedef struct {
    Ptr ptr;
    NativeFunq function;
} PtrNative;

struct PtrString {
    Ptr ptr;
    int length;
    char* chars;
    uint32_t hash;
};

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
    PtrString* name;
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
    PtrQlosure* member;
} PtrMethod;

typedef struct {
    Ptr ptr;
    NativeMethod member;
} PtrNativeMethod;

typedef struct {
    Ptr ptr;
    Value target;
    NativeMethod* member;
} PtrTargetedNativeMethod;


Ptr* allocatePtr(size_t size, PtrType type);
PtrQlosure* newClosure(PtrFunq* function);
PtrFunq* newFunction();
PtrQlass* newClass(PtrString* name);
PtrInstance* newInstance(PtrQlass* qlass);
PtrMethod* newBoundMethod(Value target, PtrQlosure* member);

PtrNative* newNative(NativeFunq function);
PtrNativeMethod* newNativeMethod(NativeMethod member);
PtrTargetedNativeMethod* newTargetedNativeMethod(Value target, PtrNativeMethod* member);

PtrString* takeString(char* chars, int length);
PtrString* copyString(const char* chars, int length);
PtrPreval* newUpvalue(Value* slot);
void printObject(Value value);

static inline bool matchPtrType(Value value, PtrType type) {
    return IS_PTR(value) && AS_PTR(value)->type == type;
}

#endif
