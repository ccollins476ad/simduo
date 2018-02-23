#ifndef H_SIM_UCIFY_
#define H_SIM_UCIFY_

#include <inttypes.h>
struct os_mbuf;

int sim_ucifi_tx(const uint8_t *dst_addr, struct os_mbuf *om, int8_t rssi);

#endif
