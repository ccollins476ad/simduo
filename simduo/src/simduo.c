#include <assert.h>
#include <stdio.h>
#include "sysinit/sysinit.h"
#include "syscfg/syscfg.h"
#include "os/os.h"
#include "native_sockets/native_sock.h"
#include "mn_socket/mn_socket.h"

#define SIMDUO_MAX_MSG_SZ       10240
#define SIMDUO_STACK_SIZE       (OS_STACK_ALIGN(512))

static bool simduo_master;
static const char *simduo_sock_path = "/tmp/simduo.sock"

struct mn_socket *simduo_socket;
static struct mn_sockaddr_un simduo_server_addr;

static struct os_task simduo_task;
static os_stack_t simduo_stack[SIMDUO_STACK_SIZE];

static struct os_eventq simduo_evq;
static struct os_mqueue simduo_rx_mq;
static struct os_mqueue simduo_tx_mq;

int
simduo_enqueue_tx(const char *json)
{
    struct os_mbuf *om;
    size_t len;
    int rc;

    om = NULL;

    len = strlen(json);
    if (len > SIMDUO_MAX_MSG_SZ) {
        rc = SYS_EINVAL;
        goto err;
    }

    om = os_msys_get_pkthdr(len, 0);
    if (om == NULL) {
        rc = SYS_ENOMEM;
        goto err;
    }

    rc = os_mbuf_append(om, json, len);
    if (rc != 0) {
        rc = SYS_ENOMEM;
        goto err;
    }

    rc = os_mqueue_put(&simduo_rsp_mq, &simduo_evq, om);
    assert(rc == 0);

    return 0;

err:
    os_mbuf_free_chain(om);
    return rc;
}

static int
simduo_process_tx(struct os_mbuf *om)
{
    int rc;

    BHD_LOG(DEBUG, "Sending %d bytes\n", OS_MBUF_PKTLEN(om));

    simduo_log_mbuf(om);
    rc = mn_sendto(simduo_socket, om,
                   (struct mn_sockaddr *)&simduo_server_addr);
    return rc;
}

static int
simduo_init_socket(void)
{
    static const union mn_socket_cb cbs = {
        .socket = {
            .readable = simduo_socket_readable,
            .writable = simduo_socket_writable,
        },
    };

    int rc;

    rc = simduo_fill_addr(&simduo_server_addr, simduo_sock_path);
    if (rc != 0) {
        return rc;
    }

    rc = mn_socket(&simduo_socket, MN_PF_LOCAL, SOCK_STREAM, 0);
    if (rc != 0) {
        return rc;
    }

    mn_socket_set_cbs(simduo_socket, NULL, &cbs);

    return 0;
}

static int
simduo_connect_slave(void)
{
    int rc;

    rc = simduo_init_socket(simduo_sock_path);
    if (rc != 0) {
        return rc;
    }

    rc = mn_connect(simduo_socket, (struct mn_sockaddr *)&simduo_server_addr);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

static int
simduo_connect_master(void)
{
    int rc;

    rc = simduo_init_socket(simduo_sock_path);
    if (rc != 0) {
        return rc;
    }

    rc = mn_bind(simduo_socket, (struct mn_sockaddr *)&simduo_server_addr);
    if (rc != 0) {
        return rc;
    }

    rc = mn_listen(simduo_socket, 1);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

void
simduo_set_master(bool master)
{
    simduo_master = master;
}

void
simduo_init(void)
{
    int rc;

    /* Ensure this function only gets called by sysinit. */
    SYSINIT_ASSERT_ACTIVE();

    if (simduo_master) {
        rc = simduo_connect_master();
    } else {
        rc = simduo_connect_slave();
    }
    SYSINIT_PANIC_ASSERT(rc == 0);
}
