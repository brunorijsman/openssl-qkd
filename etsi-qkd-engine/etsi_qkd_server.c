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
        /* Set key_handle to "NULL" key handle, to request QKD_OPEN to allocate a key handle. */
        key_handle_t key_handle = {0};
        /* TODO: For now, the stub has a hard-coded assumption that the QKD server and client run
           on the same host, and the stub never even looks at the destination address. */
        ip_address_t destination = {
            .length = 0,
            .address = {0}
        };
        /* TODO: For now, set QoS to dummy values */
        qos_t qos = {
            .requested_length = 0,   /* TODO: This one we should probably set */
            .max_bps = 0,
            .priority = 0,
            .timeout = 0
        };
        enum RETURN_CODES result;
        result = QKD_OPEN(destination, qos, key_handle);
        report_progress("QKD_OPEN", QKD_RC_SUCCESS == result);

        /* TODO: Server side processing
           - The server calls QKD_OPEN() with a NULL key_handle, which generates a new
             key_handle, which is 64 octets.   **DONE***
           - For now, we only specify the requested_lengh as a QoS parameter. (How many bytes of
             shared secret does DH require?). For now, we don't specify max_bps or priority.
           - What timeout shall we use? Is OpenSSL tolerant to this function blocking for even a few
             ms? What happens is we specify a timeout of 0 (zero)?
           - The server uses the key_handle as the DH public key. This public key will be included
             in the Server-Hello message which plays the role of the SEND_KEY_HANDLE() operation in
             the ETSI sequence diagram. Note that this is why the ETSI document requires that "no
             key material can be derived from the handle" (top of page 9).
           - The server sets the DH private key to NULL. The QKD exchange does not use any DH
             private key on the server. */
    }

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
    - The server calls QKD_CONNECT_BLOCKING. (See considerations on the client 
      side about needing the IP address of the peer, the client in this case, 
      and this call being blocking.)
    - The server calls QKD_GET_KEY which returns a key_buffer. This is used as 
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
