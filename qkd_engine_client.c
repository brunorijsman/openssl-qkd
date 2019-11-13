/**
 * qkd_engine_client.c
 * 
 * An OpenSSL Engine for OpenSSL clients that "hijacks" the existing Diffie-Helman key agreement
 * protocol in OpenSSL to implement a QKD key agreement on top of the ETSI QKD API.
 * There is a separate engine for OpenSSL servers (see qkd_engine_server.c).
 * See qkd_api.h for the definition of the ETSI QKD API.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_engine_common.h"
#include "qkd_debug.h"
#include <string.h>
#include <openssl/engine.h>

/**
 * Callback which registered in the client OpenSSL engine to be called when OpenSSL needs the engine
 * to generate a Diffie-Hellman private key and to derive the Diffie-Hellman public key from it.
 * 
 * Returns 1 on success, 0 on failure.
 */
static int client_generate_key(DH *dh)
{
    QKD_enter();

    /* On the client side, always use a fixed public key and private key. The only thing we need to
     * to generate the shared secret is the address of the server and key handle chosen by the
     * server. The server only needs the key handle (which it chose itself) and our address (which
     * it will find out when we, the client, connect to it). The server does not need any public key
     * from us. The net result is that we (the client) don't need to compute a private nor a public
     * key; we just use fixed values to give *something* to DH. */

    /* Generate the private key. Always use a fixed private key (it is not actually used for
     * anything.) */
    BIGNUM *private_key = BN_secure_new();
    if (NULL == private_key) {
        QKD_error("BN_secure_new (private_key) failed");
        QKD_return_error("%d", 0);
    }
    BN_set_word(private_key, QKD_fixed_private_key);
    QKD_debug("DH private key: %s", BN_bn2hex(private_key));

    /* Generate the public key. */
    BIGNUM *public_key = BN_secure_new();
    if (public_key == NULL) {
        QKD_error("BN_secure_new (public_key) failed");
        QKD_return_error("%d", 0);
    }
    BN_set_word(public_key, QKD_fixed_public_key);
    QKD_debug("DH public key: %s", BN_bn2hex(public_key));

    /* Return the private and public key to OpenSSL by storing them in the DH context. */
    int result = DH_set0_key(dh, public_key, private_key);
    if (result != 1) {
        QKD_error("DH_set0_key failed (return code %d)", result);
        QKD_return_error("%d", 0);
    }

    QKD_return_success("%d", 1);
}

/**
 * Callback which registered in the client OpenSSL engine to be called when OpenSSL needs the engine
 * to compute the Diffie-Hellman shared secret based on Diffie-Hellman parameters, the server public
 * key, and our own (the client's) private key.
 * 
 * Returns the size of the generated shared secret on success, -1 on failure.
 */
static int client_compute_key(unsigned char *shared_secret, const BIGNUM *public_key, DH *dh)
{
    QKD_enter();

    /* Convert the public key provided by the server into an ETSI API key handle. */
    QKD_key_handle_t key_handle = QKD_key_handle_null;
    int convert_result = QKD_bignum_to_key_handle(public_key, &key_handle);
    if (convert_result != 1) {
        QKD_error("QKD_bignum_to_key_handle failed (return code %d)", convert_result);
        QKD_return_error("%d", -1);
    }
    QKD_debug("Key handle = %s", QKD_key_handle_str(&key_handle));

    /* Use fixed QoS parameters. */
    int shared_secret_size = DH_size(dh);
    QKD_qos_t qos = {
        .requested_length = shared_secret_size,
        .max_bps = 0,
        .priority = 0,
        .timeout = 0
    };

    /* TODO: Extract the destination from the handle. For now, hard-code localhost. */
    QKD_result_t qkd_result = QKD_open("localhost", qos, &key_handle);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_open failed (return code %d)", qkd_result);
        QKD_return_error("%d", -1);
    }

    /* Connect to the QKD peer. */
    qkd_result = QKD_connect_blocking(&key_handle, 0);   /* TODO: right value for timeout? */
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_connect_blocking failed (return code %d)", qkd_result);
        QKD_return_error("%d", -1);
    }

    /* Get the QKD-generated shared secret. Note that the ETSI API wants the key to be signed chars,
     * but OpenSSL wants it to be unsigned chars. */
    qkd_result = QKD_get_key(&key_handle, (char *) shared_secret);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_get_key failed (return code %d)", qkd_result);
        QKD_return_error("%d", -1);
    }
    QKD_debug("shared secret = %s", QKD_shared_secret_str((char *) shared_secret,
                                                           shared_secret_size));

    /* Close the QKD session. */
    qkd_result = QKD_close(&key_handle);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_close failed (return code %d)", qkd_result);
        QKD_return_error("%d", -1);
    }

    QKD_return_success("%d", shared_secret_size);
}

/**
 * Initialize the engine.
 * 
 * Returns 1 on success, 0 on failure.
 */
static int client_engine_init(ENGINE *engine)
{
    QKD_enter();
    QKD_result_t qkd_result = QKD_init(false);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_init failed: ", QKD_result_str(qkd_result));
        QKD_return_error("%d", -1);
    }
    QKD_return_success("%d", 1);
}

/**
 * Bind this client engine to OpenSSL.
 * 
 * Returns 1 on success, 0 on failure.
 */
int client_engine_bind(ENGINE *engine, const char *engine_id)
{
    QKD_enter();
    int result = QKD_engine_bind(engine, "qkd_engine_client", "QKD Client Engine", client_generate_key,
                                 client_compute_key, client_engine_init);
    if (1 != result) {
        QKD_error("QKD_engine_bind failed (return code %d)", result);
        QKD_return_error("%d", 0);
    }
    QKD_return_success("%d", 1);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(client_engine_bind);
