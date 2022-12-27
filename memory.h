#ifndef sqript_memory_h
#define sqript_memory_h

#include "common.h"
#include "object.h"

#define ALLOC(T, count) (T*)reallocate(NULL, 0, sizeof(T) * (count))
#define FREE(T, ptr) reallocate(ptr, sizeof(T), 0)
#define NEXT_SIZE(v) ((v) < 0x10 ? 0x10 : (v) * 2)
#define ARR_RESIZE(T, ptr, from, to) (T*)reallocate(ptr, sizeof(T) * (from), sizeof(T) * (to))
#define ARR_FREE(T, ptr, size) reallocate(ptr, sizeof(T) * (size), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void __gcTargetPtr(Ptr* object);
void __gcTargetValue(Value value);
void __gcRunCycle();
void __freePointers();

#endif
