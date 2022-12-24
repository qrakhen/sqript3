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
    byte paramCount;
    ExtensionFunctionParam* params;
    ExtensionFunctionCallback* callback;
} ExtensionFunction;

typedef struct {
    const char* name;
    byte functionCount;
    ExtensionFunction* functions;
} Extension;

Extension* registerExtension(const char* name);
void registerExtensionFunction(Extension* extension, ExtensionFunction fn);
void addExtensionFunctionParam(ExtensionFunction* fn, ExtensionFunctionParam param);

#endif
