#ifndef sqript_memory_h
#define sqript_memory_h

#include "common.h"

#define SCALE_LIMIT(size) \
    ((size) < 8 ? 8 : (size) * 2)

#define FREE_ARRAY(type, ptr, size) \
    reallocate(ptr, sizeof(type) * (size), 0)

#define SCALE_ARRAY(type, ptr, _limit, limit) \
    (type*)reallocate(ptr, sizeof(type) * (_limit), sizeof(type) * (limit))

void* reallocate(void* ptr, size_t _size, size_t size);

#endif
