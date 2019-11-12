/**
 * qkd_api_common.c
 * 
 * Code that is common across all implementations of the ETSI QKD API, i.e. common across the mock
 * implementation and the SimulaQron implementation.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_api.h"
#include <assert.h>
#include <stdio.h>
#include <string.h> 

/**
 * Convert a QKD result code to a human readable string.
 * 
 * Returns the humand readable string.
 */
const char *QKD_result_str(QKD_result_t result)
{
    switch (result) {
        case QKD_RESULT_SUCCESS:
            return "success";
        case QKD_RESULT_SEND_FAILED:
            return "send failed";
        case QKD_RESULT_RECEIVE_FAILED:
            return "receive failed";
        case QKD_RESULT_CONNECTION_FAILED:
            return "connection failed";
        case QKD_RESULT_OUT_OF_MEMORY:
            return "out of memory";
        case QKD_STATUS_OPEN_SSL_ERROR:
            return "openssl error";
        case QKD_RESULT_NOT_SUPPORTED:
            return "not supported";
        default:
            assert(false);
    }
}

/**
 * Convert a shared secret to a human readable string.
 * 
 * Returns a pointer to the human readable string on success, NULL on failure (memory allocation
 * failed)
 * 
 * Note: returns a pointer to a string that is overwritten on the next call to this function.
 */
char *QKD_shared_secret_str(char *shared_secret, size_t shared_secret_size)
{
    static char *str = NULL;
    static size_t str_size = 0;
    size_t needed_str_size = 2 * shared_secret_size + 1;
    if (str == NULL || str_size < needed_str_size) {
        str = realloc(str, needed_str_size);
        if (!str) {
            return NULL;
        }
        str_size = needed_str_size;

    }
    char *str_p = str;
    char *shared_secret_p = shared_secret;
    for (int i = 0; i < shared_secret_size; i++) {
        snprintf(str_p, 3, "%02x", (unsigned char) *shared_secret_p);
        str_p += 2;
        shared_secret_p += 1;
    }
    *str_p = '\0';
    return str;
}

/**
 * Null key handle constant.
 */
const QKD_key_handle_t QKD_key_handle_null = {0};

/**
 * Set a key handle to the NULL value (all zeroes)
 */
void QKD_key_handle_set_null(QKD_key_handle_t *key_handle)
{
    assert(key_handle != NULL);
    bzero(key_handle, sizeof(QKD_key_handle_t));
}

/**
 * Is a key handle NULL?
 */
bool QKD_key_handle_is_null(const QKD_key_handle_t *key_handle)
{
    assert(key_handle != NULL);
    for (size_t i=0; i<sizeof(QKD_key_handle_t); i++) {
        if (key_handle->bytes[i] != 0) {
            return false;
        }
    }
    return true;
}

/**
 * Set a key handle to a random non-NULL value.
 */
void QKD_key_handle_set_random(QKD_key_handle_t *key_handle)
{
    /* Fill the handle with random bytes, but make sure we don't accidentally pick the null key. */
    assert(key_handle != NULL);
    bool at_least_one_non_zero;
    do {
        at_least_one_non_zero = false;
        for (size_t i=0; i<sizeof(QKD_key_handle_t); i++) {
            char c = rand();
            key_handle->bytes[i] = c;
            if (c) {
                at_least_one_non_zero = true;
            }
        }
    } while (!at_least_one_non_zero);
}

/**
 * Convert a key handle to a human readable string.
 * 
 * Note: returns a static string. Calling this function destroys the value returned by a previous
 * invocation.
 */
char *QKD_key_handle_str(const QKD_key_handle_t *key_handle)
{
    static char str[2 * QKD_KEY_HANDLE_SIZE + 1];
    char *p = str;
    for (int i = 0; i < QKD_KEY_HANDLE_SIZE; i++) {
        snprintf(p, 3, "%02x", (unsigned char) (key_handle->bytes[i]));
        p += 2;
    }
    *p = '\0';
    return str;
}

/**
 * Compare two key handles.
 * 
 * Returns: -1 if key_handle_1 < key_handle_2
 *           0 if key_handle_1 == key_handle_2
 *          +1 if key_handle_1 > key_handle_2
 */
int QKD_key_handle_compare(const QKD_key_handle_t *key_handle_1,
                           const QKD_key_handle_t *key_handle_2)
{
    assert(key_handle_1 != NULL);
    assert(key_handle_2 != NULL);
    return memcmp(key_handle_1->bytes, key_handle_2->bytes, QKD_KEY_HANDLE_SIZE);
}