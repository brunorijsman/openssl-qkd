#ifndef QKD_API_H
#define QKD_API_H

#include <stdbool.h>
#include <stdint.h>


/* TODO: Make sure all of these are used */
typedef enum {
    QKD_RC_SUCCESS = 0,
    QKD_RC_GET_KEY_FAILED,
    QKD_RC_NO_CONNECTION,
    QKD_RC_OPEN_FAILED,
    QKD_RC_TIMEOUT,
    QKD_RC_NOT_SUPPORTED,
} QKD_RC;

#define QKD_KEY_HANDLE_SIZE 64

typedef struct QKD_key_handle_st {
    char bytes[QKD_KEY_HANDLE_SIZE];
} QKD_key_handle_t;

extern const QKD_key_handle_t QKD_key_handle_null;

void QKD_key_handle_set_null(QKD_key_handle_t *key_handle);
bool QKD_key_handle_is_null(const QKD_key_handle_t *key_handle);
void QKD_key_handle_set_random(QKD_key_handle_t *key_handle);
char *QKD_key_handle_str(const QKD_key_handle_t *key_handle);

typedef struct QKD_qos_st {
    uint32_t requested_length;
    uint32_t max_bps;
    uint32_t priority;
    uint32_t timeout;
} QKD_qos_t;

QKD_RC QKD_open(char *destination, QKD_qos_t qos, QKD_key_handle_t *key_handle);
QKD_RC QKD_connect_nonblock(const QKD_key_handle_t *key_handle);
QKD_RC QKD_connect_blocking(const QKD_key_handle_t *key_handle, uint32_t timeout);
QKD_RC QKD_get_key(const QKD_key_handle_t *key_handle, char *key_buffer);
QKD_RC QKD_close(const QKD_key_handle_t *key_handle);

#endif
