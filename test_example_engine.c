// DEPRECATED - DELETE THIS

#include <openssl/engine.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void report_progress(const char *what, bool okay)
{
    if (okay) {
        printf("%s: OK\n", what);
    } else {
        printf("%s: FAILED\n", what);
        exit(1);
    }
}

int main(int argc, const char* argv[])
{
    OpenSSL_add_all_algorithms();
    report_progress("OpenSSL_add_all_algorithms", true);

    ERR_load_crypto_strings();
    report_progress("ERR_load_crypto_strings", true);

    ENGINE_load_dynamic();
    report_progress("ENGINE_load_dynamic", true);

    // ENGINE *engine = ENGINE_get_first();
    // while (NULL != engine) {

    //     const char *name = ENGINE_get_name(engine);
    //     report_progress("ENGINE_get_name", name != NULL);
    //     printf("name=%s\n", name);

    //     engine = ENGINE_get_next(engine);
    // }

    ENGINE *engine = ENGINE_by_id("example");
    report_progress("ENGINE_by_id", engine != NULL);

    int result = ENGINE_init(engine);
    report_progress("ENGINE_init", result != 0);

    ENGINE_finish(engine);
    report_progress("ENGINE_finish", true);

    ENGINE_free(engine);
    report_progress("ENGINE_free", true);

    return 0;
}