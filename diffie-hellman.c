#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

void report_progress(const char *what, bool okay)
{
    if (okay) {
        printf("%s: OK\n", what);
    } else {
        printf("%s: FAILED\n", what);
        exit(1);
    }
}

void init()
{
    /* Load the human readable error strings */
    ERR_load_crypto_strings();
    report_progress("ERR_load_crypto_strings", true);

    /* Load all digest and cipher algorithms */
    OpenSSL_add_all_algorithms();
    report_progress("OpenSSL_add_all_algorithms", true);

    /* Load configuration file */
    unsigned long flags = CONF_MFLAGS_IGNORE_MISSING_FILE;
    int okay = (1 == CONF_modules_load_file(NULL, NULL, flags));
    report_progress("CONF_modules_load_file", okay);
}

void diffie_hellman()
{
    /* Allocate Diffie-Hellman parameters structure */
    EVP_PKEY *params = EVP_PKEY_new();
    report_progress("EVP_PKEY_new", params != NULL);

    /* ... to be completed ... */

    /* Free Diffie-Hellman parameters structure */
    EVP_PKEY_free(params);
    report_progress("EVP_PKEY_free", true);
}

void cleanup()
{
    /* Remove all digests and ciphers */
    EVP_cleanup();
    report_progress("EVP_cleanup", true);

    /* if you omit the next, a small leak may be left when you make use of the BIO (low level API) for e.g. base64 transformations */
    CRYPTO_cleanup_all_ex_data();
    report_progress("CRYPTO_cleanup_all_ex_data", true);

    /* Remove error strings */
    ERR_free_strings();
    report_progress("ERR_free_strings", true);
}

int main(int argc, char *argv[])
{
    init();
    diffie_hellman();
    cleanup();
    return 0;
}
