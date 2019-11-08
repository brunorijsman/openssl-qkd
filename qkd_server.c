/**
 * qkd_server.c
 * 
 * An OpenSSL Engine for OpenSSL servers that "hijacks" the existing Diffie-Helman key agreement
 * protocol in OpenSSL to implement a QKD key agreement on top of the ETSI QKD API.
 * There is a separate engine for OpenSSL clients (see qkd_client.c).
 * See qkd_api.h for the definition of the ETSI QKD API.
 * 
 * (c) 2019 Bruno Rijsman, All Rights Reserved.
 * See LICENSE for licensing information.
 */

#include "qkd_engine_common.h"
#include "qkd_debug.h"
#include <string.h>
#include <openssl/engine.h>

static int server_generate_key(DH *dh)
{
    QKD_enter();
    
    BIGNUM *public_key = BN_secure_new();
    QKD_fatal_if(public_key == NULL, "BN_secure_new (public_key) failed");

    BIGNUM *private_key = BN_secure_new();
    QKD_fatal_if(private_key == NULL, "BN_secure_new (private_key) failed");

    if (QKD_return_fixed_key_for_testing) {

        QKD_report("Use fixed private public key (for testing)");

        BN_set_word(private_key, QKD_fixed_private_key);
        BN_set_word(public_key, QKD_fixed_public_key);

    } else {

        QKD_report("Use ETSI QKD API key handle as public key and private key");

        /* Use fixed QoS parameters. */
        QKD_qos_t qos = {
            .requested_length = 0,   /* TODO: This should be set */
            .max_bps = 0,
            .priority = 0,
            .timeout = 0
        };

        /* TODO: Use this style for ALL multiline comments */
        /* Call QKD_open with a null key handle. This will cause a new key handle to be allocated.
         * Set destination to NULL, which means we don't care who the remote peer is (we rely on 
         * SSL authentication). */
        QKD_key_handle_t key_handle = QKD_key_handle_null;
        QKD_RC open_result = QKD_open(NULL, qos, &key_handle);
        QKD_fatal_if(QKD_RC_SUCCESS != open_result, "QKD_open failed");
        QKD_report("Allocated key handle: %s", QKD_key_handle_str(&key_handle));

        /* Convert allocated key handle to bignum and use it as the public key as well as the
         * private key (it really doesn't matter what the private key is, as we don't use it for
         * anything.) */
        int result = QKD_key_handle_to_bignum(&key_handle, public_key);
        QKD_fatal_if(result != 1, "QKD_key_handle_to_bignum failed");
        BIGNUM *result_bn = BN_copy(private_key, public_key);
        QKD_fatal_if(result_bn == NULL, "BN_copy failed");
    }

    QKD_report("DH public key: %s", BN_bn2hex(public_key));
    QKD_report("DH private key: %s", BN_bn2hex(private_key));

    int result = DH_set0_key(dh, public_key, private_key);
    QKD_fatal_if(result == 0, "DH_set0_key failed");

    QKD_exit();
    return 1;
}

static int server_compute_key(unsigned char *key, const BIGNUM *public_key, DH *dh)
{
    QKD_enter();

    /* TODO: for now, set the shared secret to some fixed value */
    int key_size = DH_size(dh);
    memset(key, 1, key_size);
    QKD_report("shared secret = %s", QKD_shared_secret_str(key, key_size));

    /* TODO: The overloaded compute_key function on the server does the following:
    - The server verifies that the received client public key is equal to its 
      own server public key. If not, it fails. This is just a sanity check and 
      this step is not essential for guaranteeing the security of the procedure 
      (any attacker can easily spoof the public key).
    - The server calls QKD_connect_blocking. (See considerations on the client 
      side about needing the IP address of the peer, the client in this case, 
      and this call being blocking.)
    - The server calls QKD_get_key which returns a key_buffer. This is used as 
      the shared secret and returned. Note that this returns the same key_buffer 
      as that which was return on the client side, so it is indeed a shared secret.s */

    QKD_exit();
    return key_size;
}

static int server_engine_init(ENGINE *engine)
{
    QKD_enter();
    QKD_RC result = QKD_init();
    QKD_fatal_if(result != QKD_RC_SUCCESS, "QKD_init failed");
    QKD_exit();
    return 1;
}

int server_engine_bind(ENGINE *engine, const char *engine_id)
{
    return QKD_engine_bind(engine, "qkd_server", "QKD Server Engine", server_generate_key,
                           server_compute_key, server_engine_init);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(server_engine_bind);
