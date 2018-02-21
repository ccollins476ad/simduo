#include <stdlib.h>
#include <assert.h>
#include "cjson/cJSON.h"
#include "defs/error.h"

int
simduo_get_string(cJSON *parent, const char *key, char **out_value)
{
    cJSON *item;

    item = cJSON_GetObjectItem(parent, key);
    if (item == NULL) {
        return SYS_ENOENT;
    }

    if (item->type != cJSON_String) {
        return SYS_ERANGE;
    }

    *out_value = item->valuestring;
    return 0;
}

void *
malloc_success(size_t num_bytes)
{
    void *v;

    v = malloc(num_bytes);
    assert(v != NULL && "malloc returned null");

    return v;
}
