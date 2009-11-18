#ifndef _FILTERS_
#define _FILTERS_

#include "grok.h"

#ifndef _GPERF_
struct filter {
  const char *name;
  int (*func)(grok_match_t *gm, char **value, int *value_len,
              int *value_size);
};
#endif

struct filter *string_filter_lookup(const char *str, unsigned int len);
int filter_jsonencode(grok_match_t *gm, char **value, int *value_len,
                      int *value_size);
int filter_shellescape(grok_match_t *gm, char **value, int *value_len,
                       int *value_size);
int filter_shelldqescape(grok_match_t *gm, char **value, int *value_len,
                       int *value_size);

#endif /* _FILTERS_ */
