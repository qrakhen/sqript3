#ifndef sqript_common_h
#define sqript_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define sNAN_BOXING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)

#define E_ERR_SQR_COMPILE 65
#define E_ERR_SQR_RUNTIME 70
#define E_ERR_IO_NOFILE 74
#define E_ERR_IO_BADFILE 75
#define E_ERR_SYS 10

#endif
#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_STRESS_GC
#undef DEBUG_LOG_GC
