#ifndef H_SIMDUO_
#define H_SIMDUO_

struct os_mbuf;

extern int simduo_pty_fd;

void simduo_lock(void);
void simduo_unlock(void);
int simduo_write(struct os_mbuf *om);
int simduo_rx(cJSON *map);
int simduo_get_string(cJSON *parent, const char *key, char **out_value);

#endif
