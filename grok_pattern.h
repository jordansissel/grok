#ifndef _GROK_PATTERN_H_
#define _GROK_PATTERN_H_

#include "grok.h"
#include <tcutil.h>

TCLIST *grok_pattern_name_list(const grok_t *grok);

int grok_pattern_add(const grok_t *grok, const char *name, size_t name_len,
                      const char *regexp, size_t regexp_len);
int grok_pattern_find(const grok_t *grok, const char *name, size_t name_len,
                      const char **regexp, size_t *regexp_len);
int grok_patterns_import_from_file(const grok_t *grok, const char *filename);
int grok_patterns_import_from_string(const grok_t *grok, const char *buffer);

/* Exposed only for testing */
void _pattern_parse_string(const char *line,
                           const char **name, size_t *name_len,
                           const char **regexp, size_t *regexp_len);

#endif /* _GROK_PATTERN_H_ */
