#include "etsi_qkd_debug.h"    /* TODO: always own header first */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _QKD_error_if(const char *func, bool is_error, const char *msg) 
{
    if (is_error) {
        fprintf(stderr, "[%s] %s: %s\n", func, msg, strerror(errno));
        exit(1);
    }
}

void _QKD_fatal_if(const char *func, bool is_error, const char *msg) 
{
    if (is_error) {
        fprintf(stderr, "[%s] %s: %s\n", func, msg, strerror(errno));
        exit(1);
    }
}

void _QKD_report(const char *func, const char *format, ...) 
{
    va_list argptr;
    va_start(argptr, format);
    fprintf(stderr, "[%s]", func);
    vfprintf(stderr, format, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
}
