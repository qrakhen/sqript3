#include "native.h"
#include "object.h";
#include "types.h";
#include "console.h";

Register nativeMethods[512];

void registerNativeMethod(ValueType type, char* name, Byte minArgs, Byte maxArgs, NativeMethod member) {
    PtrNativeMethod* fn = newNativeMethod(member);
    fn->minArgs = minArgs;
    fn->maxArgs = maxArgs;
    registerSet(&nativeMethods[type], makeString(name, (int)strlen(name)), PTR_VAL(fn));
}

PtrTargetedNativeMethod* bindNativeMethod(Value target, String* name) {
    Value fn;
    if (!registerGet(&nativeMethods[AS_PTR(target)->type], name, &fn)) {
        return NULL;
    }
    PtrTargetedNativeMethod* method = newTargetedNativeMethod(target, (PtrNativeMethod*)AS_PTR(fn));
    return method;
}

static Value nativeToString(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    char* str = valueToString(args[0]);
    return PTR_VAL(makeString(str, strlen(str)));
}

void initNativeMethods() {
    for (int i = 0; i < 512; i++) {
        initRegister(&nativeMethods[i]);
    }

    defineNative("time", nativeTime);
    defineNative("length", nativeLength);
    defineNative("str", nativeToString);
    defineNative("clear", consoleClear);
    defineNative("run", runFile);

    registerNativeMethod(PTR_STRING, "length", 0, 0, native_StringLength);
    registerNativeMethod(PTR_STRING, "indexOf", 1, 1, native_StringIndexOf);
    registerNativeMethod(PTR_STRING, "span", 1, 2, native_StringSubString);
    registerNativeMethod(PTR_STRING, "split", 0, 1, native_StringSplit);
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

Value nativeSplit(int argCount, Value* args) {
    if (argCount != 2) return NULL_VAL;
    if (!IS_STRING(args[0])) return NULL_VAL;
    String* str = AS_STRING(args[0]);
    String* split = AS_STRING(args[1]);
    return PTR_VAL(splitString(str, split));
}

Value runFile(int argCount, Value* args) {
    if (argCount < 1) return NULL_VAL;
    if (!IS_STRING(args[0])) return NULL_VAL;
}