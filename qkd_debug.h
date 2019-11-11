/**
 * qkd_debug.h
 * 
 * Common code for debugging.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#ifndef QKD_DEBUG_H
#define QKD_DEBUG_H

#include <stdbool.h>

void _QKD_error(const char *file, int line, const char *func, const char *format, ...);

#define QKD_error(format, ...) _QKD_error(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

void _QKD_error_with_errno(const char *file, int line, const char *func, const char *format, ...);

#define QKD_error_with_errno(format, ...) _QKD_error(__FILE__, __LINE__, __func__, format, \
                                                     ##__VA_ARGS__)

void _QKD_debug(const char *file, int line, const char *func, const char *format, ...);

#define QKD_debug(format, ...) _QKD_debug(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define QKD_enter(void) QKD_debug("Enter")

#define QKD_return() QKD_debug("Return")

#define QKD_return_success(format, value) \
do { QKD_debug("Return success " format, value); return value; } while(0)

#define QKD_return_success_qkd() \
do { QKD_debug("Return success"); return QKD_RESULT_SUCCESS; } while(0)

#define QKD_return_success_void() \
do { QKD_debug("Return success"); return; } while(0)

#define QKD_return_error(format, value) \
do { QKD_error("Return error " format, value); return value; } while (0)

#define QKD_return_error_qkd(qkd_result) \
do { QKD_error("Return error %s", QKD_result_str(qkd_result)); return qkd_result; } while (0)

#endif /* QKD_DEBUG_H */
