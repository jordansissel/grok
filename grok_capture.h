#ifndef _GROK_CAPTURE_INTERNAL_H_
#define _GROK_CAPTURE_INTERNAL_H_

#include "grok_capture_xdr.h"
#include "grok.h"


void grok_capture_init(grok_t *grok, grok_capture *gct);
void grok_capture_free(grok_capture *gct);
int grok_capture_add(grok_t *grok, grok_capture *gct);
int grok_capture_get_by_id(grok_t *grok, int id, grok_capture *gct);
int grok_capture_get_by_name(grok_t *grok, const char *name, grok_capture *gct);
int grok_capture_get_by_subname(grok_t *grok, const char *subname, grok_capture *gct);
int grok_capture_get_by_capture_number(grok_t *grok, int capture_number,
                                        grok_capture *gct);

void *grok_capture_walk_init(grok_t *grok);
int grok_capture_walk_next(grok_t *grok, void *handle, grok_capture *gct);
int grok_capture_walk_end(grok_t *grok, void *handle);

int grok_capture_set_extra(grok_t *grok, grok_capture *gct, void *extra);
void _grok_capture_encode(grok_capture *gct, char **data_ret, int *size_ret);
void _grok_capture_decode(grok_capture *gct, char *data, int size);
int _grok_capture_get_db(grok_t *grok, DB *db, DBT *key, grok_capture *gct);


extern int _db_captures_by_name_key(DB *secondary, const DBT *key,
                                    const DBT *data, DBT *result);

extern int _db_captures_by_capture_number(DB *secondary, const DBT *key,
                                          const DBT *data, DBT *result);
extern int _db_captures_by_subname(DB *secondary, const DBT *key,
                                   const DBT *data, DBT *result);

#endif /* _GROK_CAPTURE_INTERNAL_H_ */
