#ifndef _GROK_DISCOVER_H_
#define _GROK_DISCOVER_H_
#include <grok.h>

typedef struct grok_discover {
  TCTREE *complexity_tree;
  grok_t *base_grok;
  unsigned int logmask;
  unsigned int logdepth;
} grok_discover_t;

grok_discover_t *grok_discover_new(grok_t *source_grok);
void grok_discover_init(grok_discover_t *gdt, grok_t *source_grok);
void grok_discover_clean(grok_discover_t *gdt);
void grok_discover_free(grok_discover_t *gdt);
void grok_discover(const grok_discover_t *gdt, grok_t *dest_grok,
                   const char *input);

#endif /* _GROK_DISCOVER_H_ */
