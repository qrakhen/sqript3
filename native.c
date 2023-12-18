#include <stdio.h>
#include <math.h>

#include "native.h"
#include "object.h";
#include "types.h";
#include "console.h";
#include "runner.h"
#include "io.h";

Register nativeMethods[1024];

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

static Value nativeSin(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

static Value nativeCos(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

static Value nativeTan(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

void initNativeMethods() {
    for (int i = 0; i < 512; i++) {
        initRegister(&nativeMethods[i]);
    }

    defineNative("time", nativeTime);
    defineNative("length", nativeLength);
    defineNative("str", nativeToString);
    defineNative("clear", consoleClear);
    defineNative("run", nativeRunFile);
    defineNative("import", nativeImport);

    defineNative("sin", nativeSin);
    defineNative("cos", nativeCos);
    defineNative("tan", nativeTan);

    registerSet(&runner.globals, makeString("PI", 2), NUMBER_VAL(3.1415926535897932384626433832795));
    registerSet(&runner.globals, makeString("radToDeg", 2), NUMBER_VAL(3.1415926535897932384626433832795 / 180));
    registerSet(&runner.globals, makeString("degToRad", 2), NUMBER_VAL(180 / 3.1415926535897932384626433832795));

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

Value nativeRunFile(int argCount, Value* args) {
    if (argCount < 1) return NULL_VAL;
    if (!IS_STRING(args[0])) return NULL_VAL;
    char* f = readFile(AS_STRING(args[0])->chars);
    interpret(AS_STRING(args[0])->chars, f);
    free(f);
    return NULL_VAL;
}

Value nativeImport(int argCount, Value* args) {
    if (argCount != 1) return NULL_VAL;
    if (!IS_STRING(args[0])) return NULL_VAL;
    char* f = readFile(AS_STRING(args[0])->chars);
    interpret(AS_STRING(args[0])->chars, f);
    free(f);
    return BOOL_VAL(true);
}

/*
Value xxx(int argCount, Value* args) {
    #include <windows.h> 
    #include <gl/GLU.h> 
    #include <gl/GL.h> 
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
        glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

        // Draw a Red 1x1 Square centered at origin
        glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        glVertex2f(-0.5f, -0.5f);    // x, y
        glVertex2f(0.5f, -0.5f);
        glVertex2f(0.5f, 0.5f);
        glVertex2f(-0.5f, 0.5f);
        glEnd();

        
}*/