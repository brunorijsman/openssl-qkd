#include <openssl/engine.h>

static const char *example_engine_id = "example";
static const char *example_engine_name = "Example Engine by Bruno Rijsman";

int example_engine_init(ENGINE *engine)
{
    return 0;
}

int example_engine_bind(ENGINE *engine, const char *engine_id)
{
    if (!ENGINE_set_id(engine, example_engine_id))
        return 0;
    
    if (!ENGINE_set_name(engine, example_engine_name))
        return 0;

    if (!ENGINE_set_init_function(engine, example_engine_init))
        return 0;

    return 1;
}

IMPLEMENT_DYNAMIC_CHECK_FN();
IMPLEMENT_DYNAMIC_BIND_FN(example_engine_bind);
