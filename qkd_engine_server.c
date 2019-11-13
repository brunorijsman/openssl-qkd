/**
 * qkd_engine_server.c
 * 
 * An OpenSSL Engine for OpenSSL servers that "hijacks" the existing Diffie-Helman key agreement
 * protocol in OpenSSL to implement a QKD key agreement on top of the ETSI QKD API.
 * There is a separate engine for OpenSSL clients (see qkd_engine_client.c).
 * See qkd_api.h for the definition of the ETSI QKD API.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_engine_common.h"
#include "qkd_debug.h"
#include <assert.h>
#include <string.h>
#include <openssl/engine.h>

/**
 * Callback which registered in the client OpenSSL engine to be called when OpenSSL needs the engine
 * to generate a Diffie-Hellman private key and to derive the Diffie-Hellman public key from it.
 * 
 * Returns 1 on success, 0 on failure.
 */
static int server_generate_key(DH *dh)
{
    QKD_enter();

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
    if (QKD_return_fixed_key_for_testing) {

        QKD_debug("Use fixed public key (for testing)");
        BN_set_word(public_key, QKD_fixed_public_key);

    } else {

        QKD_debug("Encode the ETSI QKD API key handle into the public key");

        /* TODO: Also encode the local address of the listening socket into the public key */

        /* Use fixed QoS parameters. */
        int shared_secret_size = DH_size(dh);
        QKD_qos_t qos = {
            .requested_length = shared_secret_size,
            .max_bps = 0,
            .priority = 0,
            .timeout = 0
        };

        /* Call QKD_open with a null key handle. This will cause a new key handle to be allocated.
         * Set destination to NULL, which means we don't care who the remote peer is (we rely on 
         * SSL authentication). */
        QKD_key_handle_t key_handle = QKD_key_handle_null;
        QKD_result_t qkd_result = QKD_open(NULL, qos, &key_handle);
        if (QKD_RESULT_SUCCESS != qkd_result) {
            QKD_error("QKD_open failed: %s", QKD_result_str(qkd_result));
            QKD_return_error_qkd(qkd_result);
        }
        QKD_debug("Allocated key handle: %s", QKD_key_handle_str(&key_handle));

        /* Convert allocated key handle to bignum and use it as the public key */
        QKD_key_handle_to_bignum(&key_handle, public_key);
    }

    QKD_debug("DH public key: %s", BN_bn2hex(public_key));

    int result = DH_set0_key(dh, public_key, private_key);
    if(1 != result) {
        QKD_error("DH_set0_key failed");
        QKD_return_error_qkd(QKD_STATUS_OPEN_SSL_ERROR);
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
static int server_compute_key(unsigned char *shared_secret, const BIGNUM *client_public_key, DH *dh)
{
    QKD_enter();

    /* Retrieve the key handle, which is stored in our own (i.e. the server's) public key. */
    const BIGNUM *server_public_key = NULL;
    DH_get0_key(dh, &server_public_key, NULL);   
    assert(server_public_key != NULL);
    QKD_key_handle_t key_handle = QKD_key_handle_null;
    int convert_result = QKD_bignum_to_key_handle(server_public_key, &key_handle);
    if (convert_result != 1) {
        QKD_error("QKD_bignum_to_key_handle failed (return code %d)", convert_result);
        QKD_return_error("%d", -1);
    }
    QKD_debug("Key handle = %s", QKD_key_handle_str(&key_handle));

    /* Connect to the QKD peer. We cannot do this earlier (specifically we cannot not do this in
     * server_generate_key) because the connection can only be completed *after* the client has
     * already received the TLS Server Hello message, which contains the key handle in the
     * Diffie-Hellman public key. The client needs the key handle to be able to complete the
     * connection. */
    /* TODO: right value for timeout? */
    QKD_result_t qkd_result = QKD_connect_blocking(&key_handle, 0);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_connect_blocking failed (return code %d)", convert_result);
        QKD_return_error("%d", -1);
    }

    /* Get the shared key from the QKD provider. Note that the ETSI API wants the key to be signed
     * chars, but OpenSSL wants it to be unsigned chars. */
    qkd_result = QKD_get_key(&key_handle, (char *) shared_secret);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_get_key failed (return code %d)", convert_result);
        QKD_return_error("%d", -1);
    }
    int shared_secret_size = DH_size(dh);
    QKD_debug("shared secret = %s", QKD_shared_secret_str((char *)shared_secret,
                                                           shared_secret_size));

    /* Close the QKD session. */
    qkd_result = QKD_close(&key_handle);
    if (QKD_RESULT_SUCCESS != qkd_result) {
        QKD_error("QKD_close failed (return code %d)", convert_result);
        QKD_return_error("%d", -1);
    }

    QKD_return_success("%d", shared_secret_size);
}

/**
 * Initialize the engine.
 * 
 * Returns 1 on success, 0 on failure.
 */
static int server_engine_init(ENGINE *engine)
{
    QKD_enter();
    QKD_result_t qkd_result = QKD_init(true);
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
 */int server_engine_bind(ENGINE *engine, const char *engine_id)
{
    QKD_enter();
    int result = QKD_engine_bind(engine, "qkd_engine_server", "QKD Server Engine", server_generate_key,
                                 server_compute_key, server_engine_init);
    if (1 != result) {
        QKD_error("QKD_engine_bind failed (return code %d)", result);
        QKD_return_error("%d", 0);
    }
    QKD_return_success("%d", 1);}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(server_engine_bind);
