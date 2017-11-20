#include "jso.h"

#include <stdarg.h>

// begin level
int _jso_bl(struct jso_ctx * ctx, char const * sep)
{
    if(fprintf(ctx->stream, sep) == -1)
        return 0;

    ctx->iter++;

    return 1;
}

// end level
int _jso_el(struct jso_ctx * ctx, char const * sep)
{
    if(fprintf(ctx->stream, sep) == -1)
        return 0;

    ctx->comma_bitfield = ctx->comma_bitfield & ~((uint64_t)1 << ctx->iter);
    ctx->iter--;

    return 1;
}

int _vjso_k(struct jso_ctx * ctx, char const * key, va_list ap)
{
    if(((ctx->comma_bitfield >> ctx->iter) & 1) == 1)
        if(fprintf(ctx->stream, ",") == -1)
            goto error;

    if(fprintf(ctx->stream, JSO_SEP) == -1 ||
        vfprintf(ctx->stream, key, ap) == -1 ||
        fprintf(ctx->stream, JSO_SEP) == -1 ||
        fprintf(ctx->stream, ":") == -1)
        goto error;

    ctx->comma_bitfield |= 1 << ctx->iter;

    va_end(ap);

    return 1;

    error:

    va_end(ap);

    return 0;
}

int _jso_k(struct jso_ctx * ctx, char const * key, ...)
{
    int ret;
    va_list ap;

    va_start(ap, key);
    ret = _vjso_k(ctx, key, ap);
    va_end(ap);

    return ret;
}

int _vjso_v(struct jso_ctx * ctx, char const * value, int i_str, va_list ap)
{
    if(i_str && fprintf(ctx->stream, JSO_SEP) == -1)
        return 0;

    if(vfprintf(ctx->stream, value, ap) == -1)
        return 0;

    if(i_str && fprintf(ctx->stream, JSO_SEP) == -1)
        return 0;

    return 1;
}

int _jso_v(struct jso_ctx * ctx, char const * value, int i_str, ...)
{
    int ret;
    va_list ap;

    va_start(ap, i_str);
    ret = _vjso_v(ctx, value, i_str, ap);
    va_end(ap);

    return ret;
}

int _jso_kv(struct jso_ctx * ctx, char const * key, char const * value, int is_str, ...)
{
    int ret = 1;
    va_list ap;

    va_start(ap, is_str);

    if(_jso_k(ctx, key) == -1 ||
        _vjso_v(ctx, value, is_str, ap) == -1)
        ret = 0;

    va_end(ap);

    return ret;
}

int _jso_av(struct jso_ctx * ctx, char const * value, int i_str, ...)
{
    va_list ap;

    va_start(ap, i_str);

    if(((ctx->comma_bitfield >> ctx->iter) & 1) == 1)
        if(fprintf(ctx->stream, ",") == -1)
            goto error;

    if(i_str && fprintf(ctx->stream, JSO_SEP) == -1)
        goto error;

    if(vfprintf(ctx->stream, value, ap) == -1)
        goto error;

    if(i_str && fprintf(ctx->stream, JSO_SEP) == -1)
        goto error;

    ctx->comma_bitfield |= 1 << ctx->iter;

    va_end(ap);

    return 1;

    error:

    return 0;
}
