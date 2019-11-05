#include <stdbool.h>
#include <openssl/types.h>
#include <openssl/engine.h>

static const char *example_engine_id = "example";

static const char *example_engine_name = "Example Engine by Bruno Rijsman";

/* The DH_METHOD struct definition is not provided in a public OpenSSL header? Really? */
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

static DH_METHOD example_dh_method = {
    .name = "Example DH Method",
    .generate_key = NULL,                   /* TODO */
    .bn_mod_exp = NULL,                     /* TODO */
    .init = NULL,                           /* TODO */
    .finish = NULL,                         /* TODO */
    .flags = 0,                             /* TODO */
    .app_data = NULL,                       /* TODO */
    .generate_params = NULL                 /* TODO */
};

void report_progress(const char *what, bool okay)
{
    if (okay) {
        printf("%s: OK\n", what);
    } else {
        printf("%s: FAILED\n", what);
        exit(1);
    }
}

int example_engine_init(ENGINE *engine)
{
    return 1;
}

int example_engine_bind(ENGINE *engine, const char *engine_id)
{
    int result = ENGINE_set_id(engine, example_engine_id);
    report_progress("ENGINE_set_id", result != 0);
    
    result = ENGINE_set_name(engine, example_engine_name);
    report_progress("ENGINE_set_name", result != 0);

    result = ENGINE_set_DH(engine, &example_dh_method);
    report_progress("ENGINE_set_DH", result != 0);

    result = ENGINE_set_init_function(engine, example_engine_init);
    report_progress("ENGINE_set_init_function", result != 0);

    return 1;
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(example_engine_bind);
