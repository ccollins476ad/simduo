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

char *simduo_get_string(cJSON *parent, const char *key, int *out_status);
int simduo_get_byte_string(cJSON *parent, const char *key, int max_len,
                           void *dst, int *out_len);
int simduo_get_byte_string_exact_len(cJSON *parent, const char *key, int len,
                                     void *dst);

#endif
