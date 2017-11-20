#include "jso.h"

int main(void)
{
    struct jso_ctx ctx = JSO_CTX_INIT(stdout);

    jso_bd(&ctx, error);
    jso_kvs(&ctx, error, "test", "help");
    jso_kvs(&ctx, error, "test2", "help2");
    jso_kvo(&ctx, error, "test2", "2");

    jso_k(&ctx, error, "inner");
    jso_bd(&ctx, error);
    jso_kvs(&ctx, error, "test", "help");
    jso_kvs(&ctx, error, "test", "help");
    jso_kvs(&ctx, error, "test2", "help2");
    jso_ed(&ctx, error);

    jso_k(&ctx, error, "array");
    jso_ba(&ctx, error);
    jso_avs(&ctx, error, "test1");
    jso_avs(&ctx, error, "test2");
    jso_avo(&ctx, error, "2");
    jso_ea(&ctx, error);

    jso_ed(&ctx, error);

    printf("\n");

    return 0;

    error:

    return -1;
}
