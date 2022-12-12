#ifndef sqript_log_h
#define sqript_log_h

#include "common.h"

#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 8
#define LOG_LEVEL_SPAM 16

#if OS_UNIX
#define LOG_COLOR_WHITE "\033[0;37m"
#define LOG_COLOR_RED "\033[0;31m"
#define LOG_COLOR_GREEN "\033[0;32m"
#define LOG_COLOR_YELLOW "\033[0;33m"
#define LOG_COLOR_CYAN "\033[0;36m"
#endif

void __logWrite(char* message, byte level, ...);
void logError(char* message, ...) { __logWrite(message, LOG_LEVEL_ERROR); }
void logWarn(char* message, ...) { __logWrite(message, LOG_LEVEL_WARN); }
void logInfo(char* message, ...) { __logWrite(message, LOG_LEVEL_INFO); }
void logDebug(char* message, ...) { __logWrite(message, LOG_LEVEL_DEBUG); }
void logSpam(char* message, ...) { __logWrite(message, LOG_LEVEL_SPAM); }

#endif
