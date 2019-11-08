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

    BIGNUM *public_key = BN_secure_new();
    QKD_fatal_if(public_key == NULL, "BN_secure_new (public_key) failed");

    BIGNUM *private_key = BN_secure_new();
    QKD_fatal_if(private_key == NULL, "BN_secure_new (private_key) failed");

    if (QKD_return_fixed_key_for_testing) {
        QKD_report("Return fixed key");
        BN_set_word(private_key, QKD_fixed_private_key);
        BN_set_word(public_key, QKD_fixed_public_key);
    } else {
        BN_set_word(private_key, 1);
        BN_set_word(public_key, 1);
        /* TODO: Client side processing
        - The client uses the received DH public key as the ETSI API key_handle
        - The client calls QKD_open() with the key_handle obtained it this way.
        - Note that the overloaded generate_key function does different things on the server
            side and the client side. Thus, there needs to be some way for the overloaded function
            to "know" whether it is being called in a server role or a client role. (How?)
        - The client uses the key_handle as it's own DH public key. In other words, the client's
            DH public key is the same as the server's DH public key.
        - The client sets its DH private key to NULL. Just as on the server, the QKD exchange
            does not use any DH private key on the client. */
    }

    QKD_report("DH public key: %s", BN_bn2hex(public_key));
    QKD_report("DH private key: %s", BN_bn2hex(private_key));

    int result = DH_set0_key(dh, public_key, private_key);
    QKD_fatal_if(result != 1, "DH_set0_key failed");

    QKD_exit();
    return 1;
}

static int client_compute_key(unsigned char *key, const BIGNUM *public_key, DH *dh)
{
    /* TODO: Replace all fatals with error returns (but keep report) */
    QKD_enter();
    
    // Convert the public key provided by the server into an ETSI API key handle.
    QKD_key_handle_t key_handle = QKD_key_handle_null;
    int convert_result = QKD_bignum_to_key_handle(public_key, &key_handle);
    QKD_fatal_if(convert_result != 1, "QKD_bignum_to_key_handle failed");
    QKD_report("Key handle = %s", QKD_key_handle_str(&key_handle));

    /* TODO: for now, set the shared secret to some fixed value */
    int key_size = DH_size(dh);
    memset(key, 1, key_size);
    QKD_report("shared secret = %s", QKD_shared_secret_str(key, key_size));

    // /* TODO: For now, set QoS to dummy values */
    // QKD_qos_t qos = {
    //     .requested_length = key_size,
    //     .max_bps = 0,
    //     .priority = 0,
    //     .timeout = 0
    // };
    // QKD_RC result = QKD_open("localhost", qos, &key_handle);
    // QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_open failed");

    // result = QKD_connect_blocking(&key_handle, 0);
    // QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_connect_blocking failed");
    
    // QKD_report("Allocated size=%d\n", key_size);
    // QKD_fatal_if(key == NULL, "Key is NULL");

    // // /* TODO: put somthing in the key */
    // // memset(key, 3, size);

    // result = QKD_get_key(&key_handle, (char *)key);
    // QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_get_key failed");

    // result = QKD_close(&key_handle);
    // QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_close failed");

    // /* TODO: The overloaded compute_key function on the client does the following:
    // - The client calls QKD_connect_blocking.
    // - This call requires the IP address of the peer (the server in this case). 
    //   Does OpenSSL provide some API to get the IP address of the peer?
    // - Once again, is OpenSSL tolerant to doing a blocking call here? I don't 
    //   think we can use the non-blocking variation (which is not well-defined in 
    //   the ETSI API document; there is no sequence diagram for it).
    // - The client calls QKD_get_key which returns a key_buffer. This is used as 
    //   the shared secret and returned. */

    /* TODO: Report return value in exit */
    /* TODO: Rename to QKD_DBG_... */
    QKD_exit();
    return key_size;
}

int client_engine_bind(ENGINE *engine, const char *engine_id)
{
    /* TODO: should we use init or app_data for anything? */
    /* TODO: Move the common stuff into QKD_engine_bind */
    int flags = 0;
    DH_METHOD *dh_method = DH_meth_new("ETSI QKD Client Method", flags);
    QKD_fatal_if(dh_method == NULL, "DH_new_method failed");
    int result = DH_meth_set_generate_key(dh_method, client_generate_key);
    QKD_fatal_if(result != 1, "DH_meth_set_generate_key failed");
    result = DH_meth_set_compute_key(dh_method, client_compute_key);
    QKD_fatal_if(result != 1, "DH_meth_set_compute_key failed");
    return QKD_engine_bind(engine, "qkd_client", "ETSI QKD Client Engine", dh_method);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(client_engine_bind);
