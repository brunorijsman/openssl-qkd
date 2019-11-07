#ifndef ETSI_QKD_DEBUG_H
#define ETSI_QKD_DEBUG_H

#include <stdbool.h>

void _QKD_error_if(const char *func, bool is_error, const char *msg);
void _QKD_fatal_if(const char *func, bool is_error, const char *msg);
void _QKD_report(const char *func, const char *format, ...);

#define QKD_error_if(is_error, msg) _QKD_error_if(__func__, is_error, msg)
#define QKD_fatal_if(is_error, msg) _QKD_fatal_if(__func__, is_error, msg)
#define QKD_report(format, ...) _QKD_report(__func__, format, ##__VA_ARGS__)

#endif /* ETSI_QKD_DEBUG_H */
