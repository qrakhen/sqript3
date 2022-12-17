#ifndef sqript_common_h
#define sqript_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "value.h"

#define DEBUG_MODE

static char* F(char* str, char* value) {
    char _[64];
    sprintf(_, str, value);
    char* x = &_;
    return x;
}

#endif
