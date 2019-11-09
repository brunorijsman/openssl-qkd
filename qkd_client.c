/**
 * qkd_client.c
 * 
 * An OpenSSL Engine for OpenSSL clients that "hijacks" the existing Diffie-Helman key agreement
 * protocol in OpenSSL to implement a QKD key agreement on top of the ETSI QKD API.
 * There is a separate engine for OpenSSL servers (see qkd_server.c).
 * See qkd_api.h for the definition of the ETSI QKD API.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_engine_common.h"
#include "qkd_debug.h"
#include <string.h>
#include <openssl/engine.h>

static int client_generate_key(DH *dh)
{
    QKD_enter();

    /* On the client side, always use a fixed public key and private key. The only thing we need to
     * to generate the shared secret is the address of the server and key handle chosen by the
     * server. The server only needs the key handle (which it chose itself) and our address (which
     * it will find out when we, the client, connect to it). The server does not need any public key
     * from us. The net result is that we (the client) don't need to compute a private nor a public
     * key; we just use fixed values to give *something* to DH. */
    BIGNUM *private_key = BN_secure_new();
    QKD_fatal_if(private_key == NULL, "BN_secure_new (private_key) failed");
    BN_set_word(private_key, QKD_fixed_private_key);
    QKD_report("DH private key: %s", BN_bn2hex(private_key));
    BIGNUM *public_key = BN_secure_new();
    QKD_fatal_if(public_key == NULL, "BN_secure_new (public_key) failed");
    BN_set_word(public_key, QKD_fixed_public_key);
    QKD_report("DH public key: %s", BN_bn2hex(public_key));

    int result = DH_set0_key(dh, public_key, private_key);
    QKD_fatal_if(result != 1, "DH_set0_key failed");

    QKD_exit();
    return 1;
}

static int client_compute_key(unsigned char *shared_secret, const BIGNUM *public_key, DH *dh)
{
    /* TODO: Replace all fatals with error returns (but keep report) */
    QKD_enter();

    /* Convert the public key provided by the server into an ETSI API key handle. */
    QKD_key_handle_t key_handle = QKD_key_handle_null;
    int convert_result = QKD_bignum_to_key_handle(public_key, &key_handle);
    QKD_fatal_if(convert_result != 1, "QKD_bignum_to_key_handle failed");
    QKD_report("Key handle = %s", QKD_key_handle_str(&key_handle));

    /* Use fixed QoS parameters. */
    int shared_secret_size = DH_size(dh);
    QKD_qos_t qos = {
        .requested_length = shared_secret_size,
        .max_bps = 0,
        .priority = 0,
        .timeout = 0
    };

    /* TODO: Extract the destination from the handle. For now, hard-code localhost. */
    QKD_RC qkd_result = QKD_open("localhost", qos, &key_handle);
    QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "QKD_open failed");

    /* Connect to the QKD peer. */
    qkd_result = QKD_connect_blocking(&key_handle, 0);   /* TODO: right value for timeout? */
    QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "QKD_connect_blocking failed");

    /* Get the QKD-generated shared secret. Note that the ETSI API wants the key to be signed chars,
     * but OpenSSL wants it to be unsigned chars. */
    qkd_result = QKD_get_key(&key_handle, (char *) shared_secret);
    QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "QKD_get_key failed");
    /* TODO: decide whether the caller or callee logs */
    QKD_report("shared secret = %s", QKD_shared_secret_str((char *) shared_secret,
                                                           shared_secret_size));

    /* Close the QKD session. */
    qkd_result = QKD_close(&key_handle);
    QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "QKD_close failed");

    /* TODO: Report return value in exit */
    /* TODO: Rename to QKD_DBG_... */
    QKD_exit();
    return shared_secret_size;
}

static int client_engine_init(ENGINE *engine)
{
    QKD_enter();
    QKD_exit();
    return 1;
}


int QKD_engine_bind(ENGINE *engine, const char *engine_id, const char *engine_name,
                    int (*generate_key) (DH *),
                    int (*compute_key) (unsigned char *key, const BIGNUM *pub_key, DH *dh),
                    ENGINE_GEN_INT_FUNC_PTR engine_init);

int client_engine_bind(ENGINE *engine, const char *engine_id)
{
    return QKD_engine_bind(engine, "qkd_client", "QKD Client Engine", client_generate_key,
                           client_compute_key, client_engine_init);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(client_engine_bind);
