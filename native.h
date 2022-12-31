#ifndef sqript_native_h
#define sqript_native_h

#include <time.h>

#include "register.h"
#include "value.h"
#include "object.h"
#include "array.h"

Register nativeMethods[16];

void registerNativeMethod(ValueType type, String* name, NativeMethod member);
PtrTargetedNativeMethod* bindNativeMethod(Value target, String* name);
void initNativeMethods();

Value nativeTime(int argCount, Value* args);
Value nativeLength(int argCount, Value* args);
Value nativeSubstr(int argCount, Value* args);
Value nativeSplit(int argCount, Value* args);
Value nativeIndexOf(int argCount, Value* args);

#endif