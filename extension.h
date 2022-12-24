#ifndef sqript_extension_h
#define sqript_extension_h

#include "value.h"

typedef void(*ExtensionFunctionCallback)();

typedef struct {
    const char* name;
    const ValueType type;
} ExtensionFunctionParam;

typedef struct {
    const char* name;
    const ValueType returnType;
    ExtensionFunctionParam* params;
    ExtensionFunctionCallback* callback;
} ExtensionFunction;

typedef struct {
    const char* name;
    ExtensionFunction* functions;
} Extension;

#endif
