#define _TIMESPEC_DEFINED
#define PTW32_STATIC_LIB 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "segment.h"
#include "debug.h"
#include "runner.h"
#include "console.h"
#include "io.h"
#include "thread.h"


/*

static void* myThreadFun(void* vargp) {
    printf("Printing GeeksQuiz from Thread \n");
    return NULL;
}

static  int test() {
    pthread_t thread_id;
    printf("Before Thread\n");
    pthread_create(&thread_id, NULL, myThreadFun, NULL);
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    exit(0);
}*/

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

    //runFile("./core.sqr", __SQR_DEFAULT_FLAGS);
    //runFile("./test.sqr", SQR_OPTION_FLAGS);

    if (argc == 1) {
        consoleRun(SQR_OPTION_FLAGS);
    } else if (argc >= 2) {
        runFile(argv[1], SQR_OPTION_FLAGS);
    }

    freeRunner();
    return 0;
}