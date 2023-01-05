#ifndef sqript_console_h
#define sqript_console_h

//#include <windows.h> 
//#include <ConsoleApi3.h>
//#include <consoleapi2.h>

#include "common.h"
#include "value.h"

#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 8
#define LOG_LEVEL_SPAM 16

#define COUT(m) (consoleWrite(m))
#define CERR(m) (COUTC(F("ERR: %s", m), C_COLOR_RED))
#define COUTC(m, c) (consoleWriteColor(m, c))
#define COUTLN(m) (consoleWriteLine(m))
#define COUTLNC(m, c) { consoleWriteColor(m, c); CNL; }
#define CNL (consoleWriteLine(""))
#define CSETC(c) (consoleSetColorByte(c))
#define CSETCUR(x, y) (consoleSetCursor(x, y))

#ifdef __OS_UNIX
    #define __C_ESC "\033"
#else 
    #define __C_ESC ""
#endif 

#if __OS_UNIX
#define C_COLOR_BLACK       __C_ESC "[30m"
#define C_COLOR_RED         __C_ESC "[31m"
#define C_COLOR_GREEN       __C_ESC "[32m"
#define C_COLOR_YELLOW      __C_ESC "[33m"
#define C_COLOR_BLUE        __C_ESC "[34m"
#define C_COLOR_MAGENTA     __C_ESC "[35m"
#define C_COLOR_CYAN        __C_ESC "[36m"
#define C_COLOR_LGRAY       __C_ESC "[37m"
#define C_COLOR_DGRAY       __C_ESC "[90m"
#define C_COLOR_LRED        __C_ESC "[91m"
#define C_COLOR_LGREEN      __C_ESC "[92m"
#define C_COLOR_LYELLOW     __C_ESC "[93m"
#define C_COLOR_LBLUE       __C_ESC "[94m"
#define C_COLOR_LMAGENTA    __C_ESC "[95m"
#define C_COLOR_LCYAN       __C_ESC "[96m"
#define C_COLOR_WHITE       __C_ESC "[97m"

#define C_BACKC_BLACK       __C_ESC "[40m"
#define C_BACKC_RED         __C_ESC "[41m"
#define C_BACKC_GREEN       __C_ESC "[42m"
#define C_BACKC_YELLOW      __C_ESC "[43m"
#define C_BACKC_BLUE        __C_ESC "[44m"
#define C_BACKC_MAGENTA     __C_ESC "[45m"
#define C_BACKC_CYAN        __C_ESC "[46m"
#define C_BACKC_LGRAY       __C_ESC "[47m"
#define C_BACKC_DGRAY       __C_ESC "[100m"
#define C_BACKC_LRED        __C_ESC "[101m"
#define C_BACKC_LGREEN      __C_ESC "[102m"
#define C_BACKC_LYELLOW     __C_ESC "[103m"
#define C_BACKC_LBLUE       __C_ESC "[104m"
#define C_BACKC_LMAGENTA    __C_ESC "[105m"
#define C_BACKC_LCYAN       __C_ESC "[106m"
#define C_BACKC_WHITE       __C_ESC "[107m"

#define C_COLOR_BOLD        __C_ESC "[1m"
#define C_COLOR_UNDERLINE   __C_ESC "[4m"
#define C_COLOR_R_UNDERLINE __C_ESC "[24m"
#define C_COLOR_NEGATIVE    __C_ESC "[7m"
#define C_COLOR_POSITIVE    __C_ESC "[27m"
#define C_COLOR_RESET       __C_ESC "[0m"

typedef struct {
    char* color;
    char* background;
    short cursorX;
    short cursorY;
} Console;

#endif

#ifdef __OS_WIN

#define C_COLOR_BLACK       0
#define C_COLOR_BLUE        1
#define C_COLOR_GREEN       2
#define C_COLOR_CYAN        3
#define C_COLOR_RED         4
#define C_COLOR_MAGENTA     5
#define C_COLOR_YELLOW      6
#define C_COLOR_LGRAY       7
#define C_COLOR_DGRAY       8
#define C_COLOR_LBLUE       9
#define C_COLOR_LGREEN      10
#define C_COLOR_LCYAN       11
#define C_COLOR_LRED        12
#define C_COLOR_LMAGENTA    13
#define C_COLOR_LYELLOW     14
#define C_COLOR_WHITE       15
#define C_COLOR_RESET       15

#define C_COLOR_AS_BACKGROUND(color) (color * 16)

typedef struct {
    Byte color;
    short cursorX;
    short cursorY;
} Console;

#else

#define __OS_NOT_SUPPORTED

#endif


void consoleInit();
void consoleRun(int flags);
void consoleSetColor(Byte color);
void consoleSetColorByte(Byte color);
void consoleSetBackground(Byte color);
void consoleSetCursor(short x, short y);
void consoleWrite(char* message); //va_list args);
void consoleWriteLine(char* message); //va_list args);
void consoleWriteColor(char* message, Byte color);
void consoleClear();
void consoleResetColor();

void logError(char* message, ...);
void logWarn(char* message, ...);
void logInfo(char* message, ...);
void logDebug(char* message, ...);
void logSpam(char* message, ...);

#endif
