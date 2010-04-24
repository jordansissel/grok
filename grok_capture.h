#ifndef _GROK_CAPTURE_INTERNAL_H_
#define _GROK_CAPTURE_INTERNAL_H_

#include "grok_capture_xdr.h"
#include "grok.h"


void grok_capture_init(grok_t *grok, grok_capture *gct);
void grok_capture_free(grok_capture *gct);

void grok_capture_add(grok_t *grok, const grok_capture *gct);
const grok_capture *grok_capture_get_by_id(grok_t *grok, int id);
const grok_capture *grok_capture_get_by_name(grok_t *grok, const char *name);
const grok_capture *grok_capture_get_by_subname(grok_t *grok,
                                                const char *subname);
const grok_capture *grok_capture_get_by_capture_number(grok_t *grok,
                                                       int capture_number);

void grok_capture_walk_init(grok_t *grok);
const grok_capture *grok_capture_walk_next(grok_t *grok);

int grok_capture_set_extra(grok_t *grok, grok_capture *gct, void *extra);
void _grok_capture_encode(grok_capture *gct, char **data_ret, int *size_ret);
void _grok_capture_decode(grok_capture *gct, char *data, int size);


#endif /* _GROK_CAPTURE_INTERNAL_H_ */
