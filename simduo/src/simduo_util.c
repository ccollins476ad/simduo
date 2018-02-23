#include <stdlib.h>
#include <assert.h>
#include "cjson/cJSON.h"
#include "defs/error.h"
#include "parse/parse.h"

static void
simduo_set_status(int *status, int val)
{
    if (status != NULL) {
        *status = val;
    }
}

char *
simduo_get_string(cJSON *parent, const char *key, int *out_status)
{
    cJSON *item;

    item = cJSON_GetObjectItem(parent, key);
    if (item == NULL) {
        simduo_set_status(out_status, SYS_ENOENT);
        return NULL;
    }

    if (item->type != cJSON_String) {
        simduo_set_status(out_status, SYS_ERANGE);
        return NULL;
    }

    simduo_set_status(out_status, 0);
    return item->valuestring;
}

int
simduo_get_byte_string(cJSON *parent, const char *key, int max_len,
                       void *dst, int *out_len)
{
    char *s;
    int rc;

    s = simduo_get_string(parent, key, &rc);
    if (rc != 0) {
        return rc;
    }

    return parse_byte_stream(s, max_len, dst, out_len);
}

int
simduo_get_byte_string_exact_len(cJSON *parent, const char *key, int len,
                                 void *dst)
{
    char *s;
    int rc;

    s = simduo_get_string(parent, key, &rc);
    if (rc != 0) {
        return rc;
    }

    return parse_byte_stream_exact_length(s, dst, len);
}

void *
malloc_success(size_t num_bytes)
{
    void *v;

    v = malloc(num_bytes);
    assert(v != NULL && "malloc returned null");

    return v;
}
