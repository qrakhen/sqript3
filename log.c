#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "log.h"

static byte logLevel = LOG_LEVEL_SPAM;

void __logWrite(char* message, byte level, ...) {
    if (level > logLevel)
        return;

    char buffer[32];
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    strftime(buffer, 64, "%H:%M:%S", timeInfo);
    printf(buffer);

    /*va_list args;
    va_start(args, 16);
    for (int i = 0;;i++) {
        void* x = va_arg(args, i);
        if (x == NULL)
            break;
    }*/
    printf(" > %s\n", message);
}

void logLn() {
    printf("\n");
}
