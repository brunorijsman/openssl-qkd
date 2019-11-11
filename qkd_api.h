/**
 * qkd_api.h
 * 
 * The ETSI Quantum Key Distribution (QKD) Application Programming Interface (API) as defined in
 * ETSI GS QKD 004 V.1.1.1 (2012-12).
 * https://www.etsi.org/deliver/etsi_gs/QKD/001_099/004/01.01.01_60/gs_qkd004v010101p.pdf
 * This repository contains two imlpementations of this API:
 * (1) A mock implementation (see qkd_api_mock.c)
 * (2) TODO: A simulated BB84 implementation on top of SimulaQron (see qkd_api_bb84_simulaqron.c)
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#ifndef QKD_API_H
#define QKD_API_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    QKD_RESULT_SUCCESS = 0,
    QKD_RESULT_SEND_FAILED,
    QKD_RESULT_RECEIVE_FAILED,
    QKD_RESULT_CONNECTION_FAILED,
    QKD_RESULT_OUT_OF_MEMORY,
    QKD_STATUS_OPEN_SSL_ERROR,
    QKD_RESULT_NOT_SUPPORTED
} QKD_result_t;

const char *QKD_result_str(QKD_result_t result);

#define QKD_KEY_HANDLE_SIZE 64

typedef struct QKD_key_handle_st {
    char bytes[QKD_KEY_HANDLE_SIZE];
} QKD_key_handle_t;

extern const QKD_key_handle_t QKD_key_handle_null;

char *QKD_shared_secret_str(char *shared_secret, size_t shared_secret_size);

void QKD_key_handle_set_null(QKD_key_handle_t *key_handle);
bool QKD_key_handle_is_null(const QKD_key_handle_t *key_handle);
void QKD_key_handle_set_random(QKD_key_handle_t *key_handle);
char *QKD_key_handle_str(const QKD_key_handle_t *key_handle);
int QKD_key_handle_compare(const QKD_key_handle_t *key_handle_1,
                           const QKD_key_handle_t *key_handle_2);

typedef struct QKD_qos_st {
    uint32_t requested_length;
    uint32_t max_bps;
    uint32_t priority;
    uint32_t timeout;
} QKD_qos_t;

/* API functions as described in the ESTI QKD API document. */
QKD_result_t QKD_open(char *destination, QKD_qos_t qos, QKD_key_handle_t *key_handle);
QKD_result_t QKD_connect_nonblock(const QKD_key_handle_t *key_handle);
QKD_result_t QKD_connect_blocking(const QKD_key_handle_t *key_handle, uint32_t timeout);
QKD_result_t QKD_get_key(const QKD_key_handle_t *key_handle, char *key_buffer);
QKD_result_t QKD_close(const QKD_key_handle_t *key_handle);

/* Additional API functions that are not mentioned in the ESTI API document. */
QKD_result_t QKD_init(bool am_server);
/* TODO: Also add QKD_finish function and register it in OpenSSL using ENGINE_set_finish_function */

#endif
