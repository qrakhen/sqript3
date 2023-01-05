#ifndef sqript_object_h
#define sqript_object_h

#include "common.h"
#include "segment.h"
#include "register.h"
#include "value.h"

#define ALLOCATE_PTR(type, objectType) \
    (type*)allocatePtr(sizeof(type), objectType)

// is the same as value type now so :shrug:
#define PTR_TYPE(value)     (AS_PTR(value)->type)

#define IS_METHOD(value)    matchPtrType(value, PTR_METHOD)
#define IS_QLASS(value)     matchPtrType(value, PTR_QLASS)
#define IS_QONTEXT(value)   matchPtrType(value, PTR_QLOSURE)
#define IS_FUNQ(value)      matchPtrType(value, PTR_FUNQ)
#define IS_OBJEQT(value)    matchPtrType(value, PTR_OBJEQT)
#define IS_NATIVE(value)    matchPtrType(value, PTR_NATIVE_FUNQ)

#define AS_METHOD(value)    ((Method*)AS_PTR(value))
#define AS_TNMETHOD(value)  ((PtrTargetedNativeMethod*)AS_PTR(value))
#define AS_QLASS(value)     ((Qlass*)AS_PTR(value))
#define AS_QONTEXT(value)   ((Qontext*)AS_PTR(value))
#define AS_FUNQ(value)      ((Funqtion*)AS_PTR(value))
#define AS_OBJEQT(value)    ((Objeqt*)AS_PTR(value))
#define AS_NATIVE(value)    (((NativeQall*)AS_PTR(value))->function)

typedef enum {
    PTR = T_PTR,
    PTR_METHOD = T_PTR_METHOD,
    PTR_QLASS = T_PTR_QLASS,
    PTR_QLOSURE = T_PTR_QLOSURE,
    PTR_FUNQ = T_PTR_FUNQ,
    PTR_OBJEQT = T_PTR_INSTANCE,
    PTR_ARRAY = T_PTR_ARRAY,
    PTR_NATIVE = T_PTR_NATIVE,
    PTR_NATIVE_FUNQ = T_PTR_NATIVE_FUNQ,
    PTR_NATIVE_METHOD = T_PTR_NATIVE_METHOD,
    PTR_STRING = T_PTR_STRING,
    PTR_PREVAL = T_PTR_PREVAL
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
} Funqtion;

typedef struct {
    Ptr ptr;
    NativeFunq function;
} NativeQall;

typedef struct PtrPreval {
    Ptr ptr;
    Value* location;
    Value closed;
    struct PtrPreval* next;
} PtrPreval;

typedef struct {
    Ptr ptr;
    Funqtion* function;
    PtrPreval** upvalues;
    int revalCount;
} Qontext;

typedef struct {
    Ptr ptr;
    String* name;
    Register methods;
    Register properties;
} Qlass;

typedef struct {
    Ptr ptr;
    Qlass* qlass;
    Register fields;
} Objeqt;

typedef struct {
    Ptr ptr;
    Value target;
    Qontext* method;
} Method;

typedef struct {
    Ptr ptr;
    Byte minArgs;
    Byte maxArgs;
    NativeMethod callback;
} PtrNativeMethod;

typedef struct {
    Ptr ptr;
    Value target;
    PtrNativeMethod* method;
} PtrTargetedNativeMethod;

Ptr* allocatePtr(size_t size, PtrType type);
Qontext* newClosure(Funqtion* function);
Funqtion* newFunction();
Qlass* newClass(String* name);
Objeqt* newInstance(Qlass* qlass);
Method* newBoundMethod(Value target, Qontext* member);

NativeQall* newNative(NativeFunq function);
PtrNativeMethod* newNativeMethod(NativeMethod member);
PtrTargetedNativeMethod* newTargetedNativeMethod(Value target, PtrNativeMethod* member);

PtrPreval* newUpvalue(Value* slot);
void printObject(Value value);

static inline bool matchPtrType(Value value, PtrType type) {
    return IS_PTR(value) && AS_PTR(value)->type == type;
}

#endif
