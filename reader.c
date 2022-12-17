#include <stdio.h>

#include "common.h"
#include "reader.h"

typedef struct {
    const char* start;
    const char* cursor;
    int line;
} Reader;

Reader reader;

void initReader(const char* source) {
    reader.start = source;
    reader.cursor = source;
    reader.line = 0;
}
