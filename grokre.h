#ifndef _GROKRE_H_
#define _GROKRE_H_

void grok_init(grok_t *grok);
void grok_clone(grok_t *dst, grok_t *src);
void grok_free(grok_t *grok);

int grok_compile(grok_t *grok, const char *pattern);
int grok_compilen(grok_t *grok, const char *pattern, int length);
int grok_exec(grok_t *grok, const char *text, grok_match_t *gm);
int grok_execn(grok_t *grok, const char *text, int textlen, grok_match_t *gm);

int grok_match_get_named_substring(const grok_match_t *gm, const char *name,
                                   const char **substr, int *len);

#endif /* _GROKRE_H_ */
