#include "etsi_qkd_common.h"
#include <string.h>
#include <openssl/engine.h>
#include "qkd_api.h"

static int server_generate_key(DH *dh)
{
    printf("server_generate_key [enter]\n");
    
    BIGNUM *pub_key = BN_secure_new();
    report_progress("server_generate_key: BN_secure_new (priv_key)", pub_key != NULL);

    BIGNUM *priv_key = BN_secure_new();
    report_progress("server_generate_key: BN_secure_new (pub_key)", priv_key != NULL);

    if (return_fixed_key_for_testing) {
        printf("server_generate_key (fixed number)\n");
        BN_set_word(priv_key, 11111);
        BN_set_word(pub_key, 22222);
    } else {
        printf("server_generate_key (server)\n");
        /* For now, we only specify the requested_lengh as a QoS parameter. For now, we don't
        specify max_bps or priority. */
        QKD_QOS qos = {
            .requested_length = 0,   /* TODO: This should be set */
            .max_bps = 0,
            .priority = 0,
            .timeout = 0
        };

        /* Calls QKD_open() with a NULL key_handle, which generates a new key_handle, which is 64 octets. */
        QKD_RC result;
        key_handle_t key_handle = key_handle_null;
        result = QKD_open("localhost", qos, &key_handle);
        report_progress("QKD_open", QKD_RC_SUCCESS == result);
        printf("sizeof key_handle: %ld\n", sizeof(key_handle));
        printf("key_handle: ");
        for (int i=0; i<sizeof(key_handle); i++) {
            printf("%02x", (unsigned char) (key_handle[i]));
        }
        printf("\n");

        /* The server uses the key_handle as the DH public key. This public key will be included
        in the Server-Hello message which plays the role of the SEND_KEY_HANDLE() operation in
        the ETSI sequence diagram. Note that this is why the ETSI document requires that "no
        key material can be derived from the handle" (top of page 9) */
        BN_bin2bn((const unsigned char *) key_handle, sizeof(key_handle), pub_key);

        /* The server sets the DH private key to 1. The QKD exchange does not use any DH
        private key on the server. */
        BN_set_word(priv_key, 1);
    }

    printf("DH public key: %s\n", BN_bn2hex(pub_key));
    printf("DH private key: %s\n", BN_bn2hex(priv_key));

    int result = DH_set0_key(dh, pub_key, priv_key);
    report_progress("dh_generate_key: DH_set0_key", result == 1);

    printf("server_generate_key [exit]\n");
    return 1;
}

static int server_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
    printf("server_compute_key [enter]\n");

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

    int size = DH_size(dh);
    printf("Allocated size=%d\n", size);
    report_progress("server_compute_key: allocate shared secret memory", key != NULL);
    /* TODO: put somthing in the key */
    memset(key, 3, size);

    printf("server_compute_key [exit]\n");
    return size;
}

static DH_METHOD server_dh_method = {
    .name = "ETSI QKD Server Method",
    .generate_key = server_generate_key,
    .compute_key = server_compute_key,
    .bn_mod_exp = NULL,
    .init = NULL,
    .finish = NULL,
    .flags = 0,
    .app_data = NULL,
    .generate_params = NULL
};

int server_engine_bind(ENGINE *engine, const char *engine_id)
{
    return engine_bind_common(engine, 
                              "etsi_qkd_server",
                              "ETSI QKD Server Engine",
                              &server_dh_method);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(server_engine_bind);
