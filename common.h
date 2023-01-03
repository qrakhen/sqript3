#ifndef sqript_common_h
#define sqript_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define __DBG_STACK
//#define __DBG_TRACE
//#define __DBG_GCLOG

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

#endif
