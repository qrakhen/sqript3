#ifndef sqript_common_h
#define sqript_common_h

#include <time.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define OS_WIN true
#define OS_UNIX false

#define __DBG_PRINT_STATEMENTS true
#define __DBG_PRINT_EXEC_TIME true
#define __DBG_STACK true
#define __DBG_TRACE false
#define __DBG_GCLOG false

#define __CLI_SHOW_TIME false

#define BYTE_MAX 0x100

#define E_ERR_SQR_RUNTIME 200
#define E_ERR_SQR_RUNTIME_NULLREF 201
#define E_ERR_SQR_DIGEST 100
#define E_ERR_SQR_COMPILE_SYNTAX 101
#define E_ERR_IO_NOFILE 50
#define E_ERR_IO_BADFILE 51
#define E_ERR_SYS 10
#define E_ERR 1

#define SQR_OPTION_FLAG_NOFLAGS 0
#define SQR_OPTION_FLAG_DEBUGGER 1
#define SQR_OPTION_FLAG_INTERACTIVE 2
#define SQR_OPTION_FLAG_SAFE_MODE 4
#define SQR_OPTION_FLAG_UNIX_MODE 8
#define SQR_OPTION_FLAG_MODULE_MODE 16
#define SQR_OPTION_FLAG_FORCE_TYPES 32
#define SQR_OPTION_FLAG_NO_ALIASES 64
#define SQR_OPTION_FLAG_COMPILE_ONLY 128
#define SQR_OPTION_FLAG_CONTINUE_ON_PANIC 256
#define SQR_OPTION_FLAG_COLORED_CLI 512

#define TIME_UNIT_NS 0
#define TIME_UNIT_MS 1
#define TIME_UNIT_S 2
#define TIME_UNIT_M 3
#define TIME_UNIT_H 4

#define NOW ((double)clock() / CLOCKS_PER_SEC)
#define NOW_MS ((double)clock() / ((double)CLOCKS_PER_SEC / 1000))
#define NOW_NS ((double)clock() / ((double)CLOCKS_PER_SEC / 1000 / 1000))

#define F formatToString

char* formatToString(char* format, ...);
char* formatTime(double ms, int unit);

#endif
