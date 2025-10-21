#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static Byte l2c[0x10] = { 
    C_COLOR_RED,
    C_COLOR_LRED, 
    C_COLOR_YELLOW,
    C_COLOR_WHITE,
    C_COLOR_LGRAY,
    C_COLOR_DGRAY
};

void setLogLevel(Byte level) {
    logLevel = level;
}

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
    int offset = 0;
    bool insideString = false;
    int r = 0;
    do {
        consoleWrite(__C_PREFIX_INPUT);
        if (!fgets(line + offset, sizeof(line), stdin)) {
            consoleWriteLine("");
            break;
        }
        if (!insideString && memcmp(line, "#read", 5) == 0) {
            int length = getCharLength(line + 6, '\n');
            char file[256];
            memcpy(file, line + 6, length);
            struct stat _buffer;
            if (stat(file, &_buffer) < 0)
                continue;
            char* src = readFile(file);
            r = (int)interpret(file, src);
        } else {
            for (int i = offset; i < sizeof(line); i++) {
                if (line[i] == '"') {
                    insideString = !insideString;
                }
                if (line[i] == '\n') {
                    if (!insideString) {
                        offset = 0;
                        r = (int)interpret(MODULE_DEFAULT, line);
                        break;
                    } else {
                        line[++i] = '\n';
                        offset = i;
                        break;
                    }
                }
            }
        }
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

    #if __CLI_SHOW_TIME
        char buffer[32];
        time_t t;
        struct tm * timeInfo;
        time(&t);
        timeInfo = localtime(&t);
        strftime(buffer, 32, "%H:%M:%S", timeInfo);
        printf(buffer);
    #endif

    Byte _color = console.color;
    CSETC(l2c[level]);
    char prefix[6];
    if (level < LOG_LEVEL_ERROR)
        strcpy(prefix, "FATAL");
    else if (level < LOG_LEVEL_WARN)
        strcpy(prefix, "ERROR");
    else if (level < LOG_LEVEL_INFO)
        strcpy(prefix, "WARN ");
    else if (level < LOG_LEVEL_DEBUG)
        strcpy(prefix, "INFO ");
    else if (level < LOG_LEVEL_SPAM)
        strcpy(prefix, "DEBUG");
    else 
        strcpy(prefix, "SPAM ");

    char* formatted = formatToString(format, args);
    print(formatToString("[%s]: %s\n", prefix, formatted));
    CSETC(_color);
}

void logMessage(Byte level, char* message, va_list args) { __logWrite(message, level, args); }
void logError(char* message, va_list args) { __logWrite(message, LOG_LEVEL_ERROR, args); }
void logWarn(char* message, va_list args) { __logWrite(message, LOG_LEVEL_WARN, args); }
void logInfo(char* message, va_list args) { __logWrite(message, LOG_LEVEL_INFO, args); }
void logDebug(char* message, va_list args) { __logWrite(message, LOG_LEVEL_DEBUG, args); }
void logSpam(char* message, va_list args) { __logWrite(message, LOG_LEVEL_SPAM, args); }

void logLn() {
    printf("\n");
}
