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

/* TODO: Turn debug on or off using environment variable */
static bool debug = true;

static void print_location(const char *file, int line, const char *func)
{
    fprintf(stderr, "[%s:%d (%s)] ", file, line, func);
}

void _QKD_error(const char *file, int line, const char *func, const char *format, ...) 
{
    va_list args;
    print_location(file, line, func);
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void _QKD_error_with_errno(const char *file, int line, const char *func, const char *format, ...) 
{
    va_list args;
    print_location(file, line, func);
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, ": %s\n", strerror(errno));
    va_end(args);
}

void _QKD_debug(const char *file, int line, const char *func, const char *format, ...) 
{
    if (debug) {
        va_list args;
        print_location(file, line, func);
        va_start(args, format);
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
}
