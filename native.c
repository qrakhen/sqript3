#include "native.h"

Register nativeMethods[16];

void registerNativeMethod(ValueType type, PtrString* name, NativeMethod member) {
    PtrNativeMethod* fn = newNativeMethod(member);
    registerSet(&nativeMethods[type], name, PTR_VAL(fn));
}

PtrTargetedNativeMethod* bindNativeMethod(Value target, PtrString* name) {
    Value fn;
    if (!registerGet(&nativeMethods[target.type], name, &fn)) {
        return NULL;
    }
    PtrTargetedNativeMethod* method = newTargetedNativeMethod(target, (PtrNativeMethod*)AS_PTR(fn));
    return method;
}

void initNativeMethods() {
    for (int i = 0; i < 16; i++)
        initRegister(&nativeMethods[i]);
}

Value nativeTime(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value nativeLength(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    if (matchPtrType(args[0], PTR_STRING)) return NUMBER_VAL(AS_STRING(args[0])->length);
    if (matchPtrType(args[0], PTR_ARRAY)) return NUMBER_VAL(AS_ARRAY(args[0])->length);
    else return NULL_VAL;
}

Value nativeStr(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    return PTR_VAL(valueToString(args[0]));
}