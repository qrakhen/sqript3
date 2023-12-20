#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "segment.h"
#include "debug.h"
#include "runner.h"
#include "console.h"
#include "string.h"
#include "io.h"

#ifdef __OS_WIN
    #include <windows.h>
#endif

#define __C_PREFIX_INPUT " <: "
#define __C_PREFIT_OUTPUT " :> "

static Byte logLevel = LOG_LEVEL_SPAM;
Console console;
HANDLE hOut, hCin;

static void setCursor(short x, short y) {
    printf("%c[%d;%dH", 27, x, y);
}

static void print(char* message) {
    console.cursorX += strlen(message);
    printf("%s", message);
}

static void updateColor() {
    #ifdef __OS_UNIX
        print(console.background);
        print(console.color);
    #else
        SetConsoleTextAttribute(hOut, console.color);
    #endif
}

void consoleRun(int flags) {
    printf("\n    ...made with <3 by qrakhen ~\n\n");
    char line[8192];
    int r = 0;
    do {
        consoleWrite(__C_PREFIX_INPUT);
        if (!fgets(line, sizeof(line), stdin)) {
            consoleWriteLine("");
            break;
        }
        r = (int)interpret(MODULE_DEFAULT, line);
    } while ((flags & SQR_OPTION_FLAG_SAFE_MODE) == 0 || r == 0);
}

void consoleInit() {
    #ifdef __OS_UNIX
        console.background = C_COLOR_BLACK;
        console.color = C_COLOR_WHITE;
    #else
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        hCin = GetStdHandle(STD_INPUT_HANDLE);
        console.color = C_COLOR_WHITE | (16 * C_COLOR_BLACK);
    #endif
    console.cursorX = 0;
    console.cursorY = 0;    
    updateColor();
}

void consoleSetColorByte(Byte color) {
    console.color = color;
    updateColor();
}

void consoleSetColor(Byte color) {
    console.color = (console.color & 0xF0) | color;
    updateColor();
}

void consoleSetBackground(Byte color) {
    console.color = (console.color & 0x0F) | (color * 16);
    updateColor();
}

void consoleSetCursor(short x, short y) {
    console.cursorX = x;
    console.cursorY = y;
}

void consoleWrite(char* message) {
    print(message);
}

void consoleWriteFormatted(char* format, ...) {
    char r[8192];
    va_list ptr;
    va_start(ptr, format);
    print(formatToString(format, ptr));
    va_end(ptr);
}

void consoleWriteColor(char* message, Byte color) {
    Byte _color = console.color;
    #ifdef __OS_UNIX
        print(color);
    #else 
        consoleSetColor(color);
    #endif;
    consoleWrite(message);
    consoleSetColor(_color);
}

void consoleWriteLine(char* message) {
    consoleWrite(message);
    print("\n");
    console.cursorY++;
    console.cursorX = 0;
}

void consoleResetColor() {
    console.color = C_COLOR_WHITE;
    updateColor();
}

void consoleClear() {
    #if __ACTIVE_OS_UNIX
        system("clear");
    #else
        system("cls");
    #endif
}

static void __logWrite(char* format, Byte level, va_list args) {
    if (level > logLevel)
        return;

    char buffer[32];
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    strftime(buffer, 32, "%H:%M:%S", timeInfo);
    #if __CLI_SHOW_TIME
        printf(buffer);
    #endif

    print(formatToString(format, args));
}

void logError(char* message, va_list args) { __logWrite(message, LOG_LEVEL_ERROR, args); }
void logWarn(char* message, va_list args) { __logWrite(message, LOG_LEVEL_WARN, args); }
void logInfo(char* message, va_list args) { __logWrite(message, LOG_LEVEL_INFO, args); }
void logDebug(char* message, va_list args) { __logWrite(message, LOG_LEVEL_DEBUG, args); }
void logSpam(char* message, va_list args) { __logWrite(message, LOG_LEVEL_SPAM, args); }

void logLn() {
    printf("\n");
}
