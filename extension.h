#ifndef sqript_extension_h
#define sqript_extension_h

#include <stdarg.h>

#include "value.h"

typedef Value(*ExtensionFunctionCallback)(void* caller, ...);

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
