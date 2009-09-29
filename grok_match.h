#ifndef _GROK_MATCH_H_
#define _GROK_MATCH_H_

#include "grok_capture_xdr.h"

typedef struct grok_match {
  grok_t *grok;
  const char *subject;
  int start;
  int end;
} grok_match_t;

int grok_match_get_named_capture(const grok_match_t *gm,
                                 const char *name, grok_capture *gct);
int grok_match_get_named_substring(const grok_match_t *gm, const char *name,
                                   const char **substr, int *len);

void *grok_match_walk_init(const grok_match_t *gm);
int grok_match_walk_next(const grok_match_t *gm, void *handle,
                         char **name, int *namelen,
                         const char **substr, int *substrlen);
int grok_match_walk_end(const grok_match_t *gm, void *handle);

#endif /*  _GROK_MATCH_H_ */
