#ifndef sqript_digester_h
#define sqript_digester_h

#include "object.h"
#include "runner.h"

ObjFunction* digest(const char* source);
void markCompilerRoots();

#endif
