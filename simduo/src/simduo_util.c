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
