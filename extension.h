#ifndef sqript_extension_h
#define sqript_extension_h

#include "value.h"

typedef Value(*ExtensionFunctionCallback)(int argCount, Value* args);

typedef struct {
    const char* name;
    const ValueType type;
} ExtensionFunctionParam;

typedef struct {
    const char* name;
    const ValueType returnType;
    Byte paramCount;
    ExtensionFunctionParam* params;
    ExtensionFunctionCallback* callback;
} ExtensionFunction;

typedef struct {
    const char* name;
    short functionCount;
    ExtensionFunction* functions;
} Extension;

Extension* registerExtension(const char* name);
void registerExtensionFunction(Extension* extension, ExtensionFunction fn);
void registerExtensionFunctionParam(ExtensionFunction* fn, ExtensionFunctionParam param);
Value getParamValue(Value* args, int index);

#endif
