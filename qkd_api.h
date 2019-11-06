#ifndef QKD_API_H
#define QKD_API_H

#include <stdint.h>

#define KEY_HANDLE_SIZE 64
#define IP_ADDR_MAX_LEN 16

enum RETURN_CODES {
    QKD_RC_SUCCESS = 0,
    QKD_RC_GET_KEY_FAILED = 1,
    QKD_RC_NO_CONNECTION = 2,
    QKD_RC_OPEN_FAILED = 3,
    QKD_RC_TIMEOUT_ERROR = 4,
};

typedef char key_handle_t[KEY_HANDLE_SIZE];

typedef struct {
    uint32_t requested_length;
    uint32_t max_bps;
    uint32_t priority;
    uint32_t timeout;
} qos_t;

typedef struct {
    uint8_t length;
    char address[IP_ADDR_MAX_LEN];
} ip_address_t;

uint32_t QKD_OPEN(ip_address_t destination, qos_t QoS, key_handle_t key_handle);

uint32_t QKD_CONNECT_NONBLOCK(key_handle_t key_handle, uint32_t timeout);

uint32_t QKD_CONNECT_BLOCKING(key_handle_t key_handle, uint32_t timeout);

uint32_t QKD_GET_KEY(key_handle_t key_handle, char *key_buffer);

uint32_t QKD_CLOSE(key_handle_t key_handle);

#endif
