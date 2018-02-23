#include <stdlib.h>
#include <assert.h>
#include "os/os.h"
#include "defs/error.h"
#include "base64/base64.h"
#include "dli/dli.h"
#include "simduo/simduo.h"
#include "sim_ucifi/sim_ucifi.h"

static const char *sim_ucifi_proto = "ucifi";

static int
sim_ucifi_extract_payload(cJSON *map, struct os_mbuf **out_om)
{
    const char *payload_b64;
    char *payload_raw;
    struct os_mbuf *om;
    int payload_len;
    int rc;

    payload_raw = NULL;
    *out_om = NULL;

    payload_b64 = simduo_get_string(map, "payload", &rc);
    if (rc != 0) {
        goto done;
    }

    payload_len = base64_decode_len(payload_b64);
    payload_raw = malloc(payload_len);
    if (payload_raw == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    base64_decode(payload_b64, payload_raw);

    om = os_msys_get_pkthdr(payload_len, 0);
    if (om == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    rc = os_mbuf_append(om, payload_raw, payload_len);
    if (rc != 0) {
        goto done;
    }

    *out_om = om;
    om = NULL;
    rc = 0;

done:
    free(payload_raw);
    os_mbuf_free_chain(om);
    return rc;
}

static void
sim_ucifi_rx(cJSON *map, void *arg)
{
    struct os_mbuf *om;
    uint8_t src_addr[8];
    int rc;

    rc = sim_ucifi_extract_payload(map, &om);
    if (rc != 0) {
        goto done;
    }

    rc = simduo_get_byte_string_exact_len(map, "src_addr", 8, src_addr);
    if (rc != 0) {
        goto done;
    }

    dli_rx(om, src_addr);
    om = NULL;

done:
    os_mbuf_free_chain(om);
}

static int
sim_ucifi_add_payload(cJSON *map, const struct os_mbuf *om)
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
sim_ucifi_add_rssi(cJSON *map, int8_t rssi)
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
sim_ucifi_tx(const uint8_t *dst_addr, struct os_mbuf *om, int8_t rssi)
{
    cJSON *map;
    int rc;

    map = cJSON_CreateObject();
    if (map == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    rc = sim_ucifi_add_payload(map, om);
    if (rc != 0) {
        goto done;
    }

    rc = sim_ucifi_add_rssi(map, rssi);
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
sim_ucifi_init(void)
{
    simduo_rx_dispatch_add(sim_ucifi_proto, sim_ucifi_rx, NULL);
}
