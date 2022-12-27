#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "console.h"
#include "common.h"

static Byte logLevel = LOG_LEVEL_SPAM;

Console console;

static void __setCursor(short x, short y) {
    printf("%c[%d;%dH", 27, x, y);
}

void consoleInit() {
    console.background = C_COLOR_RESET;
    console.color = C_COLOR_RESET;
    console.cursorX = 0;
    console.cursorY = 0;
}

void consoleSetColor(const char* color) {
    console.color = color;
}

void consoleSetBackground(const char* color) {
    console.background = color;
}

void consoleSetCursor(short x, short y) {
    console.cursorX = x;
    console.cursorY = y;
}

void consoleWrite(char* message) {
    printf(console.background);
    printf(console.color);
    printf(message);
    printf(C_COLOR_RESET);
    console.cursorX += strlen(message);
}

void consoleWriteColor(char* message, const char* color) {
    printf(color);
    printf(message);
    printf(C_COLOR_RESET);
}

void consoleWriteLine(char* message) {
    consoleWrite(message);
    printf("\n");
    console.cursorY++;
    console.cursorX = 0;
}

void consoleResetColor() {
    console.color = C_COLOR_RESET;
    console.background = C_COLOR_RESET;
}

void consoleClear() {
    #if OS_UNIX
    system("clear");
    #else
    system("cls");
    #endif
}

static void __logWrite(char* message, Byte level, ...) {
    if (level > logLevel)
        return;

    char buffer[32];
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    strftime(buffer, 32, "%H:%M:%S", timeInfo);
    //printf(buffer);

    /*va_list args;
    va_start(args, 16);
    for (int i = 0;;i++) {
        void* x = va_arg(args, i);
        if (x == NULL)
            break;
    }*/
    printf(" > %s\n", message);
}

void logError(char* message, ...) { __logWrite(message, LOG_LEVEL_ERROR); }
void logWarn(char* message, ...) { __logWrite(message, LOG_LEVEL_WARN); }
void logInfo(char* message, ...) { __logWrite(message, LOG_LEVEL_INFO); }
void logDebug(char* message, ...) { __logWrite(message, LOG_LEVEL_DEBUG); }
void logSpam(char* message, ...) { __logWrite(message, LOG_LEVEL_SPAM); }

void logLn() {
    printf("\n");
}
