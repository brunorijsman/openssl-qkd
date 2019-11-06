#include <string.h>
#include <openssl/engine.h>
#include "qkd_api.h"
#include "etsi_qkd_common.h"

static int client_generate_key(DH *dh)
{
    printf("client_generate_key [enter]\n");
    
    BIGNUM *pub_key = BN_secure_new();
    report_progress("client_generate_key: BN_secure_new (priv_key)", pub_key != NULL);

    BIGNUM *priv_key = BN_secure_new();
    report_progress("client_generate_key: BN_secure_new (pub_key)", priv_key != NULL);

    if (return_fixed_key_for_testing) {
        /* TODO: Move this to common */
        printf("client_generate_key (fixed number)\n");
        BN_set_word(priv_key, 11111);
        BN_set_word(pub_key, 22222);
    } else {
        /* TODO: Client side processing
        - The client uses the received DH public key as the ETSI API key_handle
        - The client calls QKD_OPEN() with the key_handle obtained it this way.
        - Note that the overloaded generate_key function does different things on the server
            side and the client side. Thus, there needs to be some way for the overloaded function
            to "know" whether it is being called in a server role or a client role. (How?)
        - The client uses the key_handle as it's own DH public key. In other words, the client's
            DH public key is the same as the server's DH public key.
        - The client sets its DH private key to NULL. Just as on the server, the QKD exchange
            does not use any DH private key on the client. */
    }

    int result = DH_set0_key(dh, pub_key, priv_key);
    report_progress("dh_generate_key: DH_set0_key", result == 1);

    printf("client_generate_key [exit]\n");
    return 1;
}

static int client_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
    printf("client_compute_key [enter]\n");
    
    /* TODO: The overloaded compute_key function on the client does the following:
    - The client calls QKD_CONNECT_BLOCKING.
    - This call requires the IP address of the peer (the server in this case). 
      Does OpenSSL provide some API to get the IP address of the peer?
    - Once again, is OpenSSL tolerant to doing a blocking call here? I don't 
      think we can use the non-blocking variation (which is not well-defined in 
      the ETSI API document; there is no sequence diagram for it).
    - The client calls QKD_GET_KEY which returns a key_buffer. This is used as 
      the shared secret and returned. */

    int size = DH_size(dh);
    printf("Allocated size=%d\n", size);
    report_progress("client_compute_key: allocate shared secret memory", key != NULL);
    /* TODO: put somthing in the key */
    memset(key, 3, size);

    printf("client_compute_key [exit]\n");
    return size;
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
                              "etsi_qkd_client",
                              "ETSI QKD Client Engine",
                              &client_dh_method);
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(client_engine_bind);
