#ifndef sqript_common_h
#define sqript_common_h

#include <time.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//#define __OS_NONE
//#define __OS_UNIX
//#define __OS_ANDROID
//#define __OS_IOS
//#define __OS_INTEGRATED
//#define __OS_UNKNOWN

#define __OS_WIN

#define __OS_ARCH_86    1
#define __OS_ARCH_64    2
#define __OS_ARCH_ARM   3

#define __DBG_PRINT_STATEMENTS true
#define __DBG_PRINT_EXEC_TIME true
#define __DBG_SHOW_FULL_INFO false
#define __DBG_STACK true
#define __DBG_TRACE false
#define __DBG_REGISTER false
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
#define E_ERR_SYS_NOT_SUPPORTED 11
#define E_ERR 1

#define MODULE_DEFAULT "__main"

#define SQR_OPTION_FLAG_NOFLAGS 0
#define SQR_OPTION_FLAG_DEBUGGER 1
#define SQR_OPTION_FLAG_INTERACTIVE 2
#define SQR_OPTION_FLAG_SAFE_MODE 4
#define SQR_OPTION_FLAG_UNIX_MODE 8
#define SQR_OPTION_FLAG_MODULE_MODE 16
#define SQR_OPTION_FLAG_FORCE_TYPES 32
#define SQR_OPTION_FLAG_NO_ALIASES 64
#define SQR_OPTION_FLAG_COMPILE_ONLY 128
#define SQR_OPTION_FLAG_READ_ONLY 256
#define SQR_OPTION_FLAG_CONTINUE_ON_PANIC 512
#define SQR_OPTION_FLAG_COLORED_CLI 1024
#define SQR_OPTION_FLAG_ENABLED(flag) (SQR_OPTION_FLAGS & flag > 0)

int SQR_OPTION_FLAGS;

#define SQR_DBG_FLAG_PRINT_STATEMENTS 1
#define SQR_DBG_FLAG_PRINT_EXEC_TIME 2
#define SQR_DBG_FLAG_SHOW_FULL_INFO 4
#define SQR_DBG_FLAG_STACK 8
#define SQR_DBG_FLAG_TRACE 16
#define SQR_DBG_FLAG_GCLOG 32
#define SQR_DBG_FLAG_ENABLED(flag) (SQR_DEBUG_FLAGS & flag > 0)

int SQR_DEBUG_FLAGS;

#define TIME_UNIT_NS 0
#define TIME_UNIT_MS 1
#define TIME_UNIT_S 2
#define TIME_UNIT_M 3
#define TIME_UNIT_H 4

#define NOW ((double)clock() / CLOCKS_PER_SEC)
#define NOW_MS ((double)clock() / ((double)CLOCKS_PER_SEC * 0.001))
#define NOW_NS ((double)clock() / ((double)CLOCKS_PER_SEC * 0.000001))
#define MEASURE(f) ( double t = NOW_MS; (f); printf("%.4f", NOW_MS - t); )
#define TIME ((unsigned long)time());

#define F formatToString


bool optionEnabled(int optionFlag);

char* formatToString(char* format, ...);
char* formatTime(double ms, int unit);

#endif
