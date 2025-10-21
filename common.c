#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h";

int SQR_OPTION_FLAGS = SQR_OPTION_FLAG_NOFLAGS;
int SQR_DEBUG_FLAGS = SQR_DBG_FLAG_PRINT_STATEMENTS | SQR_DBG_FLAG_PRINT_EXEC_TIME | SQR_DBG_FLAG_STACK;  

static char* TIME_UNIT[5] = { "ns", "ms", "s", "m", "h" };

char* formatTime(double v, int unit) {
    int i = unit;
    while (v >= 1000 && i < 4) {
        v /= 1000;
        i++;
    }
    char b[32]; 
    bool d = v - round(v) == v; 
    double x = round(v);
    int l = sprintf(b, v == 0 ? "<1" : d ? "%.0lf" : "%.3lf", v);
    char* r = malloc(l + 1);
    memcpy_s(r, l, b, l);
    *(r + l) = '\0';
    return strcat(r, TIME_UNIT[i]);
}

char* formatToString(char* format, ...) {
    va_list args;
    va_start(args, format);
    int required_size = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    char* r = (char*)malloc(required_size);
    if (r == NULL) {
        return NULL;
    }
    va_start(args, format);
    vsnprintf(r, required_size, format, args);
    va_end(args);
    return r;
}