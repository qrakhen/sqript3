#define _TIMESPEC_DEFINED
#define PTW32_STATIC_LIB 

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
    #ifdef __OS_NOT_SUPPORTED
        sprtinf(stderr, "%s", "OS NOT SUPPORTED");
        exit(E_ERR_SYS_NOT_SUPPORTED);
    #endif

    if (argc > 3) {
        printf("flags not supported (yet)\n");
        return 1;
    }

    initRunner();
    consoleInit();

    /*thread_t thread_id;
    printf("Before Thread\n");
    pthread_create(&thread_id, NULL, consoleRun, NULL);
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    exit(0);*/

    //runFile("./core.sqr", __SQR_DEFAULT_FLAGS);
    runFile("./test.sqr", __SQR_DEFAULT_FLAGS);

    if (argc == 1) {
        consoleRun(__SQR_DEFAULT_FLAGS);
    } else if (argc >= 2) {
        runFile(argv[1], __SQR_DEFAULT_FLAGS);
    }

    freeRunner();
    return 0;
}

