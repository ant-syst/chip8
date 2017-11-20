#include "debugger.h"
#include "tools.h"
#include "chip8.h"
#include "jso.h"

#include "cJSON/cJSON.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>

#define DEBUG_DISABLED ((void*)1)

#define DBG_SOCK_PATH "/var/run/chip8.sock"
#define MSG_LEN 1024

#define BKPTS_LEN (MEM_SIZE-MEM_START)
#define BKPTS_INDEX(ADDR)((ADDR)-MEM_START)
#define BKPTS_ADDR(INDEX)((INDEX)+MEM_START)

#define TRY(expression, res, label) \
do{                                 \
    if((expression) != (res))       \
        goto label;                 \
} while(0)

enum dbg_error {
    DBG_OK,
    DBG_NET_ERROR,
    DBG_INPUT_ERROR,
    DBG_SYS_ERR
};

enum dbg_state {
    DBG_RUNNING,
    DBG_STOPPED
};

#define DBG_STATE_TO_STR(state)     \
state == DBG_RUNNING ? "running" :  \
state == DBG_STOPPED ? "stopped" :  \
"unknown"

enum dbg_client_state {
    DBG_CLIENT_CONNECTED,
    DBG_CLIENT_DISCONNECTED
};

struct debugger {
    struct chip8 * chip;
    char breakpoints[BKPTS_LEN];
    enum dbg_state state;
    struct {
        enum dbg_client_state state;
        int sd;
    } client;
    struct {
        int sd;
    } server;
    struct {
        size_t size;
        char * buff;
        FILE * stream;
    } output;
};

static void msg_buffer_init(struct debugger * dbg, struct jso_ctx * ctx)
{
    *ctx = JSO_CTX_INIT(dbg->output.stream);
    rewind(dbg->output.stream);
}

static enum dbg_error msg_buffer_flush(struct debugger * dbg)
{
    if(fflush(dbg->output.stream) != 0)
        THROW(error, 1, "fflush");

    if(dbg->output.size != 0)
        if(send(dbg->client.sd, dbg->output.buff, dbg->output.size, 0) == -1)
            goto net_error;

    return DBG_OK;

    error:

    return DBG_SYS_ERR;

    net_error:

    return DBG_NET_ERROR;
}

/* Actions */

static enum dbg_error cont(struct debugger * dbg, struct jso_ctx * ctx)
{
    dbg->state = DBG_RUNNING;

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "cpu_state");
    jso_kvs(ctx, error, "value", "running");
    jso_ed(ctx, error);

    return DBG_OK;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error stop(struct debugger * dbg, struct jso_ctx * ctx)
{
    dbg->state = DBG_STOPPED;

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "cpu_state");
    jso_kvs(ctx, error, "value", "stopped");
    jso_ed(ctx, error);

    return DBG_OK;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error bkpt_add(struct debugger * dbg, uint16_t bkpt_addr, uint16_t index, struct jso_ctx * ctx)
{
    if(dbg->breakpoints[index])
        goto bkpt_error;

    dbg->breakpoints[index] = 1;

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "bkpt_added");
    jso_kvo(ctx, error, "addr", "%u", (unsigned int)bkpt_addr);
    jso_ed(ctx, error);

    return DBG_OK;

    bkpt_error:

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "error");
    jso_kvs(ctx, error, "value", "breakpoint_exists");
    jso_kvo(ctx, error, "addr", "%u", (unsigned int)bkpt_addr);
    jso_ed(ctx, error);

    return DBG_INPUT_ERROR;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error bkpt_rm(struct debugger * dbg, uint16_t bkpt_addr, uint16_t index, struct jso_ctx * ctx)
{
    if(!dbg->breakpoints[index])
        goto bkpt_error;

    dbg->breakpoints[index] = 0;

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "bkpt_removed");
    jso_kvo(ctx, error, "addr", "%u", (unsigned int)bkpt_addr);
    jso_ed(ctx, error);

    return DBG_OK;

    bkpt_error:

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "error");
    jso_kvs(ctx, error, "value", "breakpoint_not_found");
    jso_kvo(ctx, error, "addr", "%u", (unsigned int)bkpt_addr);
    jso_ed(ctx, error);

    return DBG_INPUT_ERROR;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error bkpt_info(struct debugger * dbg, struct jso_ctx * ctx)
{
    int i;

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "breakpoints");
    jso_k(ctx, error, "value");

    jso_ba(ctx, error);
    for(i=0; i<BKPTS_LEN; i++)
        if(dbg->breakpoints[i])
            jso_avo(ctx, error, "%d", BKPTS_ADDR(i));
    jso_ea(ctx, error);

    jso_ed(ctx, error);

    return DBG_OK;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error bkpt_triggered(struct debugger * dbg, uint16_t bkpt_addr, struct jso_ctx * ctx)
{
    UNUSED(dbg);

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "bkpt_triggered");
    jso_kvo(ctx, error, "addr", "%"PRIu16, bkpt_addr);
    jso_ed(ctx, error);

    return DBG_OK;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error cpu_info(struct debugger * dbg, struct jso_ctx * ctx)
{
    int i;
    struct cpu * c = &(dbg->chip->cpu);

    jso_bd(ctx, error);
    jso_kvs(ctx, error, "type", "cpu");
    jso_k(ctx, error, "value");

    jso_bd(ctx, error);

    jso_kvs(ctx, error, "state", "%s", DBG_STATE_TO_STR(dbg->state));
    jso_kvo(ctx, error, "it", "%u", chip8_decode_it(dbg->chip));
    jso_kvo(ctx, error, "i", "%d", (int)c->i);
    jso_kvo(ctx, error, "pc", "%d", (int)c->pc);
    jso_kvo(ctx, error, "sp", "%d", (int)c->sp);
    jso_kvo(ctx, error, "dt", "%d", (int)c->dt);
    jso_kvo(ctx, error, "st", "%d", (int)c->st);

    jso_k(ctx, error, "v");
    jso_ba(ctx, error);
    for(i=0; i<NUM_GPR; i++)
        jso_avo(ctx, error, "%d", c->v[i]);
    jso_ea(ctx, error);

    jso_ed(ctx, error);

    jso_ed(ctx, error);

    return DBG_OK;

    error:

    return DBG_SYS_ERR;
}

static enum dbg_error exec_action(struct debugger * dbg)
{
    ssize_t len;
    char message[MSG_LEN];
    struct jso_ctx ctx;
    cJSON * root = NULL, * type, * value;
    uint16_t addr, index;
    enum dbg_error err;

    len = recv(dbg->client.sd, message, sizeof(message)-1, 0);
    if(len == -1 || len == 0)
        goto fatal_error;
    else
        message[len] = '\0';

    root = cJSON_Parse(message);
    type = cJSON_GetObjectItemCaseSensitive(root, "type");
    if(!cJSON_IsString(type))
        goto input_error;

    msg_buffer_init(dbg, &ctx);

    if(strcmp(type->valuestring, "bkpt_rm") == 0 || strcmp(type->valuestring, "bkpt_add") == 0)
    {
        value = cJSON_GetObjectItemCaseSensitive(root, "addr");
        if(!cJSON_IsNumber(value))
            goto input_error;
        addr = value->valueint;

        if(!chip8_check_it_addr(addr))
            goto wrong_addr_error;

        index = BKPTS_INDEX(addr);
    }

    if(strcmp(type->valuestring, "bkpt_rm") == 0)
        err = bkpt_rm(dbg, addr, index, &ctx);
    else if(strcmp(type->valuestring, "bkpt_add") == 0)
        err = bkpt_add(dbg, addr, index, &ctx);
    else if(strcmp(type->valuestring, "bkpt_info") == 0)
        err = bkpt_info(dbg, &ctx);
    else if(strcmp(type->valuestring, "cpu_info") == 0)
        err = cpu_info(dbg, &ctx);
    else if(strcmp(type->valuestring, "continue") == 0)
        err = cont(dbg, &ctx);
    else if(strcmp(type->valuestring, "stop") == 0)
        err = stop(dbg, &ctx);
    else
        goto input_error;

    if(err != DBG_OK && err != DBG_INPUT_ERROR)
        goto fatal_error;

    cJSON_Delete(root);

    return msg_buffer_flush(dbg);

    input_error:

    cJSON_Delete(root);

    jso_bd(&ctx, fatal_error);
    jso_kvs(&ctx, fatal_error, "type", "error");
    jso_kvs(&ctx, fatal_error, "value", "wrong_input");
    jso_ed(&ctx, fatal_error);
    msg_buffer_flush(dbg);

    return DBG_INPUT_ERROR;

    wrong_addr_error:

    cJSON_Delete(root);

    jso_bd(&ctx, fatal_error);
    jso_kvs(&ctx, fatal_error, "type", "error");
    jso_kvs(&ctx, fatal_error, "value", "invalid_address");
    jso_kvo(&ctx, fatal_error, "addr", "%u", (unsigned int)addr);
    jso_ed(&ctx, fatal_error);
    msg_buffer_flush(dbg);

    return DBG_INPUT_ERROR;

    fatal_error:

    cJSON_Delete(root);

    return DBG_NET_ERROR;
}

static int create_server_socket(void)
{
    struct sockaddr_un addr;
    int sd;

    unlink(DBG_SOCK_PATH);

    sd = socket(PF_UNIX, SOCK_SEQPACKET, 0);
    if(sd == -1)
        THROW(error, 1, "socket");

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, DBG_SOCK_PATH, sizeof(addr.sun_path)-1);

    if(bind(sd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)
        THROW(error, 1, "bind");

    if(listen(sd, 1) == -1)
        THROW(error, 1, "listen");

    return sd;

    error:

    return 0;
}

void close_client(struct debugger * dbg)
{
    shutdown(dbg->client.sd, SHUT_RDWR);
    close(dbg->client.sd);
    dbg->client.state = DBG_CLIENT_DISCONNECTED;
}

void dbg_free(struct debugger ** dbg)
{
    if(*dbg && *dbg != DEBUG_DISABLED)
    {
        if((*dbg)->output.stream)
            fclose((*dbg)->output.stream);

        if((*dbg)->output.buff)
            free((*dbg)->output.buff);

        if((*dbg)->server.sd > 0)
            close((*dbg)->server.sd);

        if((*dbg)->client.state == DBG_CLIENT_CONNECTED)
            close_client(*dbg);

        free(*dbg);
    }

    *dbg = NULL;
}

struct debugger * dbg_alloc(struct chip8 * chip, int enable_debug)
{
    struct debugger * dbg;

    if(enable_debug)
    {
        dbg = calloc(1, sizeof(struct debugger));
        if(!dbg)
            THROW(error, 1, "calloc");

        dbg->chip = chip;
        dbg->output.stream = open_memstream(&dbg->output.buff, &dbg->output.size);
        if(!dbg->output.stream)
            THROW(error, 1, "open_memstream");

        dbg->server.sd = create_server_socket();
        if(dbg->server.sd <= 0)
            THROW(error, 0, "create_server");
        dbg->client.state = DBG_CLIENT_DISCONNECTED;
        dbg->state = DBG_STOPPED;
    }
    else
        dbg = DEBUG_DISABLED;

    return dbg;

    error:

    dbg_free(&dbg);

    return NULL;
}

static enum dbg_error msg_has_been_received(struct debugger * dbg, int * received)
{
    int ret;
    fd_set readfds;
    static struct timeval time; // zeroed struct

    *received = 0;

    FD_ZERO(&readfds);
    FD_SET(dbg->client.sd, &readfds);

    ret = select(dbg->client.sd + 1, &readfds, NULL, NULL, &time);
    if(ret == -1)
        goto net_error;

    if(ret == 1)
        *received = 1;

    return DBG_OK;

    net_error:

    return DBG_NET_ERROR;
}

static void accept_new_client(struct debugger * dbg)
{
    while(dbg->client.state == DBG_CLIENT_DISCONNECTED)
    {
        dbg->client.sd = accept(dbg->server.sd, NULL, 0);
        if(dbg->client.sd == -1)
            perror("accept");
        else
            dbg->client.state = DBG_CLIENT_CONNECTED;
    }
}

int dbg_call(struct debugger * dbg)
{
    enum dbg_error err;
    struct jso_ctx ctx;
    int received, cont = 1;

    if(dbg == DEBUG_DISABLED)
        return 1;

    do
    {
        switch(dbg->client.state)
        {
            case DBG_CLIENT_DISCONNECTED:
                accept_new_client(dbg);
            break;

            case DBG_CLIENT_CONNECTED:
            {
                switch(dbg->state)
                {
                    case DBG_STOPPED:
                        TRY(err = exec_action(dbg), DBG_OK, error);
                    break;

                    case DBG_RUNNING:
                    {
                        TRY(err = msg_has_been_received(dbg, &received), DBG_OK, error);
                        if(received)
                            TRY(err = exec_action(dbg), DBG_OK, error);

                        if(dbg->breakpoints[BKPTS_INDEX(dbg->chip->cpu.pc)])
                        {
                            dbg->state = DBG_STOPPED;

                            msg_buffer_init(dbg, &ctx);
                            TRY(err = bkpt_triggered(dbg, dbg->chip->cpu.pc, &ctx), DBG_OK, error);
                            TRY(err = msg_buffer_flush(dbg), DBG_OK, error);
                        }

                        cont = 0;
                    }
                    break;
                }
            }
            break;
        }

        error:

        if(err == DBG_NET_ERROR)
            close_client(dbg);
        else if(err == DBG_SYS_ERR)
            return 0;
    }
    while(cont);

    return 1;
}
