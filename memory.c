#include <stdlib.h>

#include "memory.h"

void* reallocate(void* ptr, size_t _size, size_t size)
{
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    void* r = realloc(ptr, size);
    if (r == NULL) exit(1);
    return r;
}
