#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h";

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
    char r[8192];
    va_list ptr;
    va_start(ptr, format);
    vsprintf(r, format, ptr);
    va_end(ptr);
    return r;
}