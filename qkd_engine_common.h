#ifndef ETSI_KQD_COMMON_H
#define ETSI_KQD_COMMON_H

#include <stdbool.h>
#include <openssl/dh.h>

/* TODO: Rename this to client_server_common */
/* TODO: Follow QKD_ naming convention */

int shared_secret_nr_bytes(DH *dh);
int engine_bind_common(ENGINE *engine, const char *engine_id, const char *engine_name, 
                       DH_METHOD *dh_method);

/* We can control a "fixed" key (instead of an actual QKD-negotiated key) to allow end-to-end
testing before the interaction with the QKD-API has actually been implemented. */
extern bool QKD_return_fixed_key_for_testing;
extern const unsigned long QKD_fixed_public_key;
extern const unsigned long QKD_fixed_private_key;

/* When we run on SimulaQron, we do certain things differently than "in real life" (see below) */
extern bool QKD_running_on_simulaqron;

/* The DH_METHOD struct definition is not provided in a public OpenSSL header */
/* TODO: Can we avoid this with a get function? */
struct dh_method {
    char *name;
    int (*generate_key) (DH *dh);
    int (*compute_key) (unsigned char *key, const BIGNUM *pub_key, DH *dh);
    int (*bn_mod_exp) (const DH *dh, BIGNUM *r, const BIGNUM *a,
                       const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx,
                       BN_MONT_CTX *m_ctx);
    int (*init) (DH *dh);
    int (*finish) (DH *dh);
    int flags;
    char *app_data;
    int (*generate_params) (DH *dh, int prime_len, int generator,
                            BN_GENCB *cb);
};

#endif