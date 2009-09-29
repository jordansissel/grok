#include "grok.h"

int grok_match_get_named_capture(const grok_match_t *gm,
                                 const char *name, grok_capture *gct) {
  int ret;
  grok_capture_init(gm->grok, gct);
  ret = grok_capture_get_by_name(gm->grok, name, gct);
  if (ret == 0) return ret;
  ret = grok_capture_get_by_subname(gm->grok, name, gct);
  return ret;
}

int grok_match_get_named_substring(const grok_match_t *gm, const char *name,
                                   const char **substr, int *len) {
  int start, end;
  grok_capture gct;

  grok_log(gm->grok, LOG_MATCH, "Fetching named capture: %s", name);
  if (grok_match_get_named_capture(gm, name, &gct) != 0) {
    grok_log(gm->grok, LOG_MATCH, "Named capture '%s' not found", name);
    *substr = NULL;
    *len = 0;
    return -1;
  }

  start = (gm->grok->pcre_capture_vector[gct.pcre_capture_number * 2]);
  end = (gm->grok->pcre_capture_vector[gct.pcre_capture_number * 2 + 1]);
  grok_log(gm->grok, LOG_MATCH, "Capture '%s' is %d -> %d of string '%s'",
           name, start, end, gm->subject);
  *substr = gm->subject + start;
  *len = (end - start);

  grok_capture_free(&gct);
  return 0;
}

void *grok_match_walk_init(const grok_match_t *gm) {
  grok_t *grok = gm->grok;
  return grok_capture_walk_init(grok);
}

int grok_match_walk_next(const grok_match_t *gm, void *handle,
                         char **name, int *namelen,
                         const char **substr, int *substrlen) {
  grok_capture gct;
  int start, end;
  int ret;
  grok_capture_init(gm->grok, &gct);
  ret = grok_capture_walk_next(gm->grok, handle, &gct);
  if (ret != 0) {
    //grok_log(gm->grok, LOG_MATCH,
             //"grok_capture_walk_next returned nonzero: %d", ret);
    return ret;
  }

  *namelen = gct.name_len; //strlen(gct.name);
  *name = malloc(*namelen);
  memcpy(*name, gct.name, *namelen);

  start = (gm->grok->pcre_capture_vector[gct.pcre_capture_number * 2]);
  end = (gm->grok->pcre_capture_vector[gct.pcre_capture_number * 2 + 1]);
  grok_log(gm->grok, LOG_MATCH, "CaptureWalk '%.*s' is %d -> %d of string '%s'",
           *namelen, *name, start, end, gm->subject);
  *substr = gm->subject + start;
  *substrlen = (end - start);
  grok_capture_free(&gct);

  return 0;
}

int grok_match_walk_end(const grok_match_t *gm, void *handle) {
  return grok_capture_walk_end(gm->grok, handle);
}
