#include <assert.h>
#include <string.h>
#include <openssl/engine.h>
#include "qkd_api.h"
#include "qkd_common.h"
#include "qkd_debug.h"

static int client_generate_key(DH *dh)
{
    QKD_enter();

    BIGNUM *pub_key = BN_secure_new();
    QKD_fatal_if(pub_key == NULL, "BN_secure_new (pub_key) failed");

    BIGNUM *priv_key = BN_secure_new();
    QKD_fatal_if(priv_key == NULL, "BN_secure_new (priv_key) failed");

    if (return_fixed_key_for_testing) {
        /* TODO: Move this to common */
        printf("client_generate_key (fixed number)\n");
        BN_set_word(priv_key, 11111);
        BN_set_word(pub_key, 22222);
    } else {
        BN_set_word(priv_key, 1);
        BN_set_word(pub_key, 1);
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

    int result = DH_set0_key(dh, pub_key, priv_key);
    QKD_fatal_if(result != 1, "DH_set0_key failed");

    QKD_exit();
    return 1;
}

static int client_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
    /* TODO: Replace all fatals with error returns (but keep report) */
    QKD_enter();
    
    // We get the key handle from the public key of the server.
    key_handle_t key_handle = key_handle_null;
    int size = BN_bn2bin(pub_key, (unsigned char *)key_handle);
    assert(size == KEY_HANDLE_SIZE);

    /* TODO: Remove this */
    printf("KEY HANDLE: ");
    int i;
    for (i = 0; i < sizeof(key_handle); i++) {
        printf("%02x", (unsigned char) (key_handle[i]));
    }
    printf("\n");

    /* TODO: For now, set QoS to dummy values */
    int key_size = DH_size(dh);
    QKD_QOS qos = {
        .requested_length = key_size,   /* TODO: This one we should probably set */
        .max_bps = 0,
        .priority = 0,
        .timeout = 0
    };
    QKD_RC result = QKD_open("localhost", qos, &key_handle);
    QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_open failed");

    result = QKD_connect_blocking(&key_handle, 0);
    QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_connect_blocking failed");
    
    QKD_report("Allocated size=%d\n", key_size);
    QKD_fatal_if(key == NULL, "Key is NULL");

    // /* TODO: put somthing in the key */
    // memset(key, 3, size);

    result = QKD_get_key(&key_handle, (char *)key);
    QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_get_key failed");

    printf("KEY: ");
    for (int i = 0; i < sizeof(key); i++) {
        printf("%02x", (unsigned char) (key[i]));
    }
    printf("\n");

    result = QKD_close(&key_handle);
    QKD_fatal_if(QKD_RC_SUCCESS != result, "QKD_close failed");

    /* TODO: The overloaded compute_key function on the client does the following:
    - The client calls QKD_connect_blocking.
    - This call requires the IP address of the peer (the server in this case). 
      Does OpenSSL provide some API to get the IP address of the peer?
    - Once again, is OpenSSL tolerant to doing a blocking call here? I don't 
      think we can use the non-blocking variation (which is not well-defined in 
      the ETSI API document; there is no sequence diagram for it).
    - The client calls QKD_get_key which returns a key_buffer. This is used as 
      the shared secret and returned. */



    /* TODO: Report return value in exit */
    /* TODO: Rename to QKD_DBG_... */
    QKD_exit();
    return key_size;
}

static DH_METHOD client_dh_method = {
    .name = "ETSI QKD Client Method",
    .generate_key = client_generate_key,
    .compute_key = client_compute_key,
    .bn_mod_exp = NULL,
    .init = NULL,
    .finish = NULL,
    .flags = 0,
    .app_data = NULL,
    .generate_params = NULL
};

int client_engine_bind(ENGINE *engine, const char *engine_id)
{
    return engine_bind_common(engine, 
                              "qkd_client",
                              "ETSI QKD Client Engine",
                              &client_dh_method);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(client_engine_bind);
