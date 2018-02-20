#include <assert.h>
#include <unistd.h>
#include "os/os.h"

static int
simduo_tx_raw(cJSON *map)
{
    text = cJSON_Print(map);
    if (text == NULL) {
        rc = SYS_ENOMEM;
        goto done;
    }

    len = strlen(text);
    rem_buf = sizeof simduo_tx_buf - simduo_tx_buf_sz;
    if (len > rem_buf) {
        rc = SYS_ENOMEM;
        goto done;
    }

    memcpy(simduo_tx_buf, text, len);
    simduo_tx_buf_sz += len;

    simduo_tx_timer_exp();

    rc = 0;

done:
    cJSON_free(text);
    cJSON_Delete(map);
    return rc;
}

int
simduo_tx(cJSON *map, int8_t rssi)
{
    int rc;

    /* XXX: Add version info, etc. */

    simduo_lock();
    rc = simduo_tx_raw(map);
    simduo_unlock();

    return rc;
}
