#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "debug.h"

char * dlToString(DebugLevel dl) {
    static char INFO [] = "INFO";
    static char ERROR [] = "ERROR";
    switch(dl) {
    default:
    case DL_INFO:
        return INFO;
    case DL_ERROR:
        return ERROR;
    }
}

void printDebugString(const char * file, int line, const char * method, DebugLevel dl, const char* format, ...) {
    va_list arglist;
    va_start(arglist, format);
    printf("%s: %s:%d:%s:", dlToString(dl), file, line, method);
    vprintf( format, arglist );
    va_end( arglist );
}


