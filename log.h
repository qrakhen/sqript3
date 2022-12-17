#ifndef sqript_log_h
#define sqript_log_h

#include "value.h"

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

void logError(char* message, ...);
void logWarn(char* message, ...);
void logInfo(char* message, ...);
void logDebug(char* message, ...);
void logSpam(char* message, ...);

#endif
