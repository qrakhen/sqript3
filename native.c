#include "native.h"
#include "types.h"

Register nativeMethods[16];

void registerNativeMethod(ValueType type, String* name, NativeMethod member) {
    PtrNativeMethod* fn = newNativeMethod(member);
    registerSet(&nativeMethods[type], name, PTR_VAL(fn));
}

PtrTargetedNativeMethod* bindNativeMethod(Value target, String* name) {
    Value fn;
    if (!registerGet(&nativeMethods[target.type], name, &fn)) {
        return NULL;
    }
    PtrTargetedNativeMethod* method = newTargetedNativeMethod(target, (PtrNativeMethod*)AS_PTR(fn));
    return method;
}

void initNativeMethods() {
    defineNative("time", nativeTime);
    defineNative("length", nativeLength);
    defineNative("substr", nativeSubstr);
    defineNative("split", nativeSplit);
    defineNative("indexOf", nativeIndexOf);

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

Value nativeSubstr(int argCount, Value* args) {
    if (argCount == 1) return args[0];
    if (!IS_STRING(args[0])) return NULL_VAL;
    String* str = AS_STRING(args[0]);
    int from = AS_NUMBER(args[1]);
    int length = argCount > 2 ? AS_NUMBER(args[2]) : ((int)str->length - from);
    return PTR_VAL(subString(str, from, length));
}

Value nativeSplit(int argCount, Value* args) {
    if (argCount != 2) return NULL_VAL;
    if (!IS_STRING(args[0])) return NULL_VAL;
    String* str = AS_STRING(args[0]);
    String* split = AS_STRING(args[1]);
    return PTR_VAL(splitString(str, split));
}

Value nativeIndexOf(int argCount, Value* args) {
    if (argCount != 2) return NULL_VAL;
    if (IS_STRING(args[0])) {
        String* str = AS_STRING(args[0]);
        String* needle = AS_STRING(args[1]);
        return stringIndexOf(str, needle);
    } else {
        return NULL_VAL;
    }
}