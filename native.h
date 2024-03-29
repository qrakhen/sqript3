#ifndef sqript_native_h
#define sqript_native_h

#include <time.h>

#include "register.h"
#include "value.h"
#include "object.h"
#include "array.h"

#define NATIVE_GET_ARG(args, index, type) (type(args[index]))

Register nativeMethods[1024];

void registerNativeMethod(ValueType type, char* name, Byte minArgs, Byte maxArgs, NativeMethod member);
PtrTargetedNativeMethod* bindNativeMethod(Value target, String* name);
void initNativeMethods();

Value nativeTime(int argCount, Value* args);
Value nativeLength(int argCount, Value* args);
Value nativeSplit(int argCount, Value* args); 
Value nativeRunFile(int argCount, Value* args);
Value nativeImport(int argCount, Value* args);

#endif
