/**
 * qkd_debug.c
 * 
 * Common code for debugging.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_debug.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: Add progress function */
/* TODO: format instead of msg everwhere */

static void print_location(const char *file, int line, const char *func)
{
    fprintf(stderr, "[%s:%d (%s)] ", file, line, func);
}

void _QKD_fatal_if(const char *file, int line, const char *func, bool is_error, const char *msg) 
{
    if (is_error) {
        print_location(file, line, func);
        fprintf(stderr, "%s\n", msg);
        exit(1);
    }
}

void _QKD_fatal_with_errno_if(const char *file, int line, const char *func, bool is_error,
                              const char *msg) 
{
    if (is_error) {
        fprintf(stderr, "%s: %s\n", msg, strerror(errno));
        exit(1);
    }
}

void _QKD_report(const char *file, int line, const char *func, const char *format, ...) 
{
    print_location(file, line, func);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
}
