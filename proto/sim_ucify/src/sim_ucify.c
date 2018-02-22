#include <stdlib.h>
#include <assert.h>
#include "os/os.h"
#include "defs/error.h"
#include "base64/base64.h"
#include "simduo/simduo.h"
#include "sim_ucify/sim_ucify.h"

static const char *sim_ucify_proto = "ucify";

static void
sim_ucify_rx(cJSON *map, void *arg)
{
    /* XXX: Pass packet up to layer 2.5. */
}

static int
sim_ucify_add_payload(cJSON *map, const struct os_mbuf *om)
{
    char *encoded;
    cJSON *obj;
    int enc_len;
    int rc;

    encoded = NULL;

    enc_len = BASE64_ENCODE_SIZE(OS_MBUF_PKTLEN(om));
    encoded = malloc(enc_len);
    if (encoded == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    rc = os_mbuf_copydata(om, 0, OS_MBUF_PKTLEN(om), encoded);
    assert(rc == 0);
    base64_encode(encoded, OS_MBUF_PKTLEN(om), encoded, 1);

    obj = cJSON_CreateString(encoded);
    if (obj == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    cJSON_AddItemToObject(map, "payload", obj);

    rc = 0;

done:
    free(encoded);
    return rc;
}

static int
sim_ucify_add_rssi(cJSON *map, int8_t rssi)
{
    cJSON *obj;

    obj = cJSON_CreateNumber(rssi);
    if (obj == NULL) {
        return SYS_ENOMEM;
    }

    cJSON_AddItemToObject(map, "rssi", obj);

    return 0;
}

int
sim_ucify_tx(const uint8_t *dst_addr, struct os_mbuf *om, int8_t rssi)
{
    cJSON *map;
    int rc;

    map = cJSON_CreateObject();
    if (map == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    rc = sim_ucify_add_payload(map, om);
    if (rc != 0) {
        goto done;
    }

    rc = sim_ucify_add_rssi(map, rssi);
    if (rc != 0) {
        goto done;
    }

    rc = simduo_tx(map);
    map = NULL;

done:
    os_mbuf_free_chain(om);
    cJSON_Delete(map);
    return rc;
}

void
sim_ucify_init(void)
{
    simduo_rx_dispatch_add(sim_ucify_proto, sim_ucify_rx, NULL);
}
