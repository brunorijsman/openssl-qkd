#ifndef QKD_API_H
#define QKD_API_H

#include <sys/types.h>

#define KEY_HANDLE_SIZE 64

/* TODO: Make sure all of these are used */
typedef enum {
    QKD_RC_SUCCESS = 0,
    QKD_RC_GET_KEY_FAILED,
    QKD_RC_NO_CONNECTION,
    QKD_RC_OPEN_FAILED,
    QKD_RC_TIMEOUT,
    QKD_RC_NOT_SUPPORTED,
} QKD_RC;

typedef char key_handle_t[KEY_HANDLE_SIZE];

#define key_handle_null {0}

typedef struct qkd_qos_t {
    uint32_t requested_length;
    uint32_t max_bps;
    uint32_t priority;
    uint32_t timeout;
} QKD_QOS;

/* TODO: More consistent type naming, uppercase or not, prefix or not, _t vs _st */

QKD_RC QKD_open(char *destination, QKD_QOS qos, key_handle_t *key_handle);

QKD_RC QKD_connect_nonblock(const key_handle_t *key_handle);

QKD_RC QKD_connect_blocking(const key_handle_t *key_handle, uint32_t timeout);

QKD_RC QKD_get_key(const key_handle_t *key_handle, char *key_buffer);

QKD_RC QKD_close(key_handle_t *key_handle);

#endif
