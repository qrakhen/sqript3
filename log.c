#include <stdio.h>
#include <time.h>

#include "log.h"

static byte logLevel = LOG_LEVEL_SPAM;

void __logInit() {
}

void __logWrite(char* message, byte level) {
    if (level > logLevel)
        return;

    char buffer[64];
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    strftime(buffer, 64, "%H:%M:%S", timeInfo);
    printf(buffer);
    printf(" > %s\n", message);
}
