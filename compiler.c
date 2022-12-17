#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "reader.h"

void compile(const char* source) {
    initReader(source);
    int line = -1;
    do {
        Token token = scanToken();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        }
        else {
            printf(" | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);
    } while (token.type != TOKEN_EOF);
}
