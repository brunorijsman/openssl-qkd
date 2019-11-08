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
        QKD_report("Return fixed key");
        BN_set_word(private_key, QKD_fixed_private_key);
        BN_set_word(public_key, QKD_fixed_public_key);
    } else {

        BN_set_word(private_key, 1);
        BN_set_word(public_key, 1);
        /* TODO */

        // QKD_report("server_generate_key (server)");
        // /* For now, we only specify the requested_lengh as a QoS parameter. For now, we don't
        // specify max_bps or priority. */
        // QKD_qos_t qos = {
        //     .requested_length = 0,   /* TODO: This should be set */
        //     .max_bps = 0,
        //     .priority = 0,
        //     .timeout = 0
        // };

        // /* Calls QKD_open() with a NULL key_handle, which generates a new key_handle, which is 64 octets. */
        // QKD_RC result;
        // QKD_key_handle_t key_handle = key_handle_null;
        // result = QKD_open("localhost", qos, &key_handle);
        // QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_open failed");
        // QKD_report("sizeof key_handle: %ld\n", sizeof(key_handle));

        // /* TODO: use convert to string and QKD_report */
        // fprintf(stderr, "key_handle: ");
        // for (int i=0; i<sizeof(key_handle); i++) {
        //     printf("%02x", (unsigned char) (key_handle[i]));
        // }
        // printf("\n");

        // /* The server uses the key_handle as the DH public key. This public key will be included
        // in the Server-Hello message which plays the role of the SEND_KEY_HANDLE() operation in
        // the ETSI sequence diagram. Note that this is why the ETSI document requires that "no
        // key material can be derived from the handle" (top of page 9) */
        // BN_bin2bn((const unsigned char *) key_handle, sizeof(key_handle), public_key);

        // /* The server sets the DH private key to 1. The QKD exchange does not use any DH
        // private key on the server. */
        // BN_set_word(private_key, 1);
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

int server_engine_bind(ENGINE *engine, const char *engine_id)
{
    /* TODO: should we use init or app_data for anything? */
    /* TODO: Move the common stuff into QKD_engine_bind */
    int flags = 0;
    DH_METHOD *dh_method = DH_meth_new("ETSI QKD Server Method", flags);
    QKD_fatal_if(dh_method == NULL, "DH_new_method failed");
    int result = DH_meth_set_generate_key(dh_method, server_generate_key);
    QKD_fatal_if(result != 1, "DH_meth_set_generate_key failed");
    result = DH_meth_set_compute_key(dh_method, server_compute_key);
    QKD_fatal_if(result != 1, "DH_meth_set_compute_key failed");
    return QKD_engine_bind(engine, "qkd_client", "ETSI QKD Server Engine", dh_method);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(server_engine_bind);
