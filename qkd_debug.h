#ifndef QKD_DEBUG_H
#define QKD_DEBUG_H

#include <stdbool.h>

void _QKD_fatal_if(const char *file, int line, const char *func, bool is_error, const char *msg);

#define QKD_fatal_if(is_error, msg) _QKD_fatal_if(__FILE__, __LINE__, __func__, is_error, msg)

void _QKD_fatal_with_errno_if(const char *file, int line, const char *func, bool is_error,
                              const char *msg);

#define QKD_fatal_with_errno_if(is_error, msg) _QKD_fatal_with_errno_if(__FILE__, __LINE__, \
                                                                        __func__, is_error, msg)

void _QKD_report(const char *file, int line, const char *func, const char *format, ...);

#define QKD_report(format, ...) _QKD_report(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define QKD_enter(void) QKD_report("Enter")

#define QKD_exit(void) QKD_report("Exit")

#endif /* QKD_DEBUG_H */
