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
#include "options.h"

/*
static void* myThreadFun(void* vargp) {
    printf("Printing x from Thread \n");
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
        
    printf("Launch options passed:\n");
    LaunchOption* options = parseLaunchOptions(argc, argv);
    for (int i = 0; i < argc - 1; i++) {
        if (options[i].info != NULL) {
            if (options[i].strValue != NULL)
                printf("%s = %s\n", options[i].info->key, options[i].strValue);
            else 
                printf("%s = %i\n", options[i].info->key, options[i].value);
        }
    }

    LaunchOption* logLevel = __GET_ARG("log-level");
    if (logLevel != NULL)
        setLogLevel(logLevel->value);
    else
        setLogLevel(LOG_LEVEL_WARN);

    initRunner(true);
    consoleInit();

    //runFile("core.sqr", SQR_OPTION_FLAGS);

    if (argc == 1) {
        consoleRun(SQR_OPTION_FLAGS);
    }
    else {
        if (__GET_ARG("file") != NULL) {            
            runFile(__GET_ARG("file")->strValue, SQR_OPTION_FLAGS);
            if (__GET_ARG("keep-alive") != NULL) {
                initRunner(false);
                consoleRun(SQR_OPTION_FLAGS);
            }
        } else {
            consoleRun(SQR_OPTION_FLAGS);
        }
    }

    freeRunner();
    return 0;
}