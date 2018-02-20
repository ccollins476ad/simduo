#ifndef H_SIMDUO_
#define H_SIMDUO_

typedef void simduo_rx_fn(cJSON *map, void *arg);

void simduo_set_master(bool master);
int simduo_tx(cJSON *map, int8_t rssi);

#endif
