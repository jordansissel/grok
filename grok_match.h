/**
 * @file grok_match.h
 */
#ifndef _GROK_MATCH_H_
#define _GROK_MATCH_H_

#include "grok_capture_xdr.h"

typedef struct grok_match {
  /** The grok_t instance that generated this match.
   * This is a pointer to the grok parameter given to grok_exec().
   * @see grok_exec()
   */
  const grok_t *grok;

  /** Pointer to the string matched.
   * This is a pointer to the text parameter given to grok_exec() 
   * @see grok_exec()
   */
  const char *subject;

  /** Start position of the match. 0 is beginning of string */
  int start;

  /** End position of match. */
  int end;
} grok_match_t;

const grok_capture * grok_match_get_named_capture(const grok_match_t *gm,
                                                  const char *name);
int grok_match_get_named_substring(const grok_match_t *gm, const char *name,
                                   const char **substr, int *len);

void grok_match_walk_init(const grok_match_t *gm);
int grok_match_walk_next(const grok_match_t *gm,
                         char **name, int *namelen,
                         const char **substr, int *substrlen);
void grok_match_walk_end(const grok_match_t *gm);

#endif /*  _GROK_MATCH_H_ */
