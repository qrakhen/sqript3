#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "segment.h"
#include "debug.h"
#include "runner.h"
#include "console.h"
#include "io.h"

uint16_t __SQR_DEFAULT_FLAGS = SQR_OPTION_FLAG_NOFLAGS;

int main(int argc, const char* argv[]) {
    if (argc > 3) {
        printf("flags not supported (yet)\n");
        return 1;
    }

    initRunner();
    consoleInit();

    runFile("./test.sqr", __SQR_DEFAULT_FLAGS);

    if (argc == 1) {
        consoleRun(__SQR_DEFAULT_FLAGS);
    } else if (argc >= 2) {
        runFile(argv[1], __SQR_DEFAULT_FLAGS);
    }

    freeRunner();
    return 0;
}

