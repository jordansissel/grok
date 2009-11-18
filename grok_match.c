#include "grok.h"

const grok_capture *grok_match_get_named_capture(const grok_match_t *gm,
                                                 const char *name) {
  const grok_capture *gct;
  gct = grok_capture_get_by_name(gm->grok, name);

  /* Try subname if by_name doesn't find anything */
  if (gct == NULL) {
    gct = grok_capture_get_by_subname(gm->grok, name);
  }
  return gct;
}

int grok_match_get_named_substring(const grok_match_t *gm, const char *name,
                                   const char **substr, int *len) {
  int start, end;
  const grok_capture *gct;

  grok_log(gm->grok, LOG_MATCH, "Fetching named capture: %s", name);
  gct = grok_match_get_named_capture(gm, name);
  if (gct == NULL) {
    grok_log(gm->grok, LOG_MATCH, "Named capture '%s' not found", name);
    *substr = NULL;
    *len = 0;
    return -1;
  }

  start = (gm->grok->pcre_capture_vector[gct->pcre_capture_number * 2]);
  end = (gm->grok->pcre_capture_vector[gct->pcre_capture_number * 2 + 1]);
  grok_log(gm->grok, LOG_MATCH, "Capture '%s' == '%.*s' is %d -> %d of string '%s'",
           name, end - start, gm->subject + start, start, end, gm->subject);
  *substr = gm->subject + start;
  *len = (end - start);

  return 0;
}

void grok_match_walk_init(const grok_match_t *gm) {
  grok_t *grok = gm->grok;
  grok_capture_walk_init(grok);
}

int grok_match_walk_next(const grok_match_t *gm,
                         char **name, int *namelen,
                         const char **substr, int *substrlen) {
  const grok_capture *gct;
  int start, end;
  gct = grok_capture_walk_next(gm->grok);
  if (gct == NULL) {
    return 1;
  }

  *namelen = gct->name_len;
  *name = malloc(*namelen);
  memcpy(*name, gct->name, *namelen);

  start = (gm->grok->pcre_capture_vector[gct->pcre_capture_number * 2]);
  end = (gm->grok->pcre_capture_vector[gct->pcre_capture_number * 2 + 1]);
  grok_log(gm->grok, LOG_MATCH, "CaptureWalk '%.*s' is %d -> %d of string '%s'",
           *namelen, *name, start, end, gm->subject);
  *substr = gm->subject + start;
  *substrlen = (end - start);

  return 0;
}

void grok_match_walk_end(const grok_match_t *gm) {
  /* nothing, anymore */
}
