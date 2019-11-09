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
#include <assert.h>
#include <string.h>
#include <openssl/engine.h>

static int server_generate_key(DH *dh)
{
    QKD_enter();

    /* Always use a fixed private key (it is not actually used for anything.) */    
    BIGNUM *private_key = BN_secure_new();
    QKD_fatal_if(private_key == NULL, "BN_secure_new (private_key) failed");
    BN_set_word(private_key, QKD_fixed_private_key);

    /* Choose a public key. */
    BIGNUM *public_key = BN_secure_new();
    QKD_fatal_if(public_key == NULL, "BN_secure_new (public_key) failed");

    if (QKD_return_fixed_key_for_testing) {

        QKD_report("Use fixed public key (for testing)");
        BN_set_word(public_key, QKD_fixed_public_key);

    } else {

        QKD_report("Encode the ETSI QKD API key handle into the public key");

        /* TODO: Also encode the local address of the listening socket into the public key */

        /* Use fixed QoS parameters. */
        /* TODO: Move this to common. */
        int shared_secret_size = DH_size(dh);
        QKD_qos_t qos = {
            .requested_length = shared_secret_size,
            .max_bps = 0,
            .priority = 0,
            .timeout = 0
        };

        /* TODO: Use this style for ALL multiline comments */
        /* Call QKD_open with a null key handle. This will cause a new key handle to be allocated.
         * Set destination to NULL, which means we don't care who the remote peer is (we rely on 
         * SSL authentication). */
        QKD_key_handle_t key_handle = QKD_key_handle_null;
        QKD_RC qkd_result = QKD_open(NULL, qos, &key_handle);
        QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "QKD_open failed");
        QKD_report("Allocated key handle: %s", QKD_key_handle_str(&key_handle));

        /* Convert allocated key handle to bignum and use it as the public key */
        int result = QKD_key_handle_to_bignum(&key_handle, public_key);
        QKD_fatal_if(result != 1, "QKD_key_handle_to_bignum failed");
    }

    QKD_report("DH private key: %s", BN_bn2hex(private_key));
    QKD_report("DH public key: %s", BN_bn2hex(public_key));

    int result = DH_set0_key(dh, public_key, private_key);
    QKD_fatal_if(result == 0, "DH_set0_key failed");

    QKD_exit();
    return 1;
}

static int server_compute_key(unsigned char *shared_secret, const BIGNUM *client_public_key, DH *dh)
{
    QKD_enter();

    /* Retrieve the key handle, which is stored in our own (i.e. the server's) public key. */
    const BIGNUM *server_public_key = NULL;
    DH_get0_key(dh, &server_public_key, NULL);
    assert(server_public_key != NULL);
    QKD_key_handle_t key_handle = QKD_key_handle_null;
    int convert_result = QKD_bignum_to_key_handle(server_public_key, &key_handle);
    QKD_fatal_if(convert_result != 1, "QKD_bignum_to_key_handle failed");
    QKD_report("Key handle = %s", QKD_key_handle_str(&key_handle));

    /* Connect to the QKD peer. We cannot do this earlier (specifically we cannot not do this in
     * server_generate_key) because the connection can only be completed *after* the client has
     * already received the TLS Server Hello message, which contains the key handle in the
     * Diffie-Hellman public key. The client needs the key handle to be able to complete the
     * connection. */
    QKD_RC qkd_result = QKD_connect_blocking(&key_handle, 0);   /* TODO: right value for timeout? */
    QKD_fatal_if(QKD_RC_SUCCESS != qkd_result, "QKD_connect_blocking failed");

    /* TODO: for now, set the shared secret to some fixed value */
    int shared_secret_size = DH_size(dh);
    memset(shared_secret, 1, shared_secret_size);
    QKD_report("shared secret = %s", QKD_shared_secret_str(shared_secret, shared_secret_size));

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
    return shared_secret_size;
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
