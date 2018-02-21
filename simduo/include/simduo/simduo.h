#ifndef H_SIMDUO_
#define H_SIMDUO_

#include <stdbool.h>
#include "cjson/cJSON.h"

#define SIMDUO_MAX_MSG_SZ       10240
#define SIMDUO_VERSION          1

typedef void simduo_rx_fn(cJSON *map, void *arg);

void simduo_set_server(bool server);
int simduo_rx_dispatch_add(const char *proto, simduo_rx_fn *cb, void *arg);
int simduo_tx(cJSON *map);

#endif
