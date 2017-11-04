#ifndef H_JSO
#define H_JSO

#include <stdint.h>
#include <stdio.h>

#define JSO_CTX_INIT(stream) (struct jso_ctx){0, 0, stream}

#define JSO_SEP "\""

#define jso_bd(ctx, error)  \
do {                        \
    if(!_jso_bl(ctx, "{"))  \
        goto error;         \
} while(0)

#define jso_ed(ctx, error)  \
do {                        \
    if(!_jso_el(ctx, "}"))  \
        goto error;         \
} while(0)

#define jso_ba(ctx, error)  \
do {                        \
    if(!_jso_bl(ctx, "["))  \
        goto error;         \
} while(0)

#define jso_ea(ctx, error)  \
do {                        \
    if(!_jso_el(ctx, "]"))  \
        goto error;         \
} while(0)

#define jso_avs(ctx, error, value, ...)         \
do {                                            \
    if(!_jso_av(ctx, value, 1, ##__VA_ARGS__))  \
        goto error;                             \
} while(0)

#define jso_avo(ctx, error, value, ...)         \
do {                                            \
    if(!_jso_av(ctx, value, 0, ##__VA_ARGS__))  \
        goto error;                             \
} while(0)

#define jso_k(ctx, error, key, ...)         \
do {                                        \
    if(!_jso_k(ctx, key, ##__VA_ARGS__))    \
        goto error;                         \
} while(0)

#define jso_kvs(ctx, error, key, value, ...)        \
do {                                                \
    if(!_jso_kv(ctx, key, value, 1, ##__VA_ARGS__)) \
        goto error;                                 \
} while(0)

#define jso_kvo(ctx, error, key, value, ...)        \
do {                                                \
    if(!_jso_kv(ctx, key, value, 0, ##__VA_ARGS__)) \
        goto error;                                 \
} while(0)

struct jso_ctx {
    uint64_t comma_bitfield;
    int iter;
    FILE * stream;
};

// begin level
int _jso_bl(struct jso_ctx * ctx, char const * sep);
// end level
int _jso_el(struct jso_ctx * ctx, char const * sep);

int _jso_k(struct jso_ctx * ctx, char const * key, ...);

int _jso_v(struct jso_ctx * ctx, char const * value, int i_str, ...);

int _jso_kv(struct jso_ctx * ctx, char const * key, char const * value, int is_str, ...);

int _jso_av(struct jso_ctx * ctx, char const * value, int i_str, ...);

#endif
