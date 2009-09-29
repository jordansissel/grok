#include "grok.h"
#include "grok_capture.h"
#include "grok_capture_xdr.h"

#include <db.h>
#include <assert.h>

#define CAPTURE_NUMBER_NOT_SET (-1)

char *EMPTYSTR = "";

void grok_capture_init(grok_t *grok, grok_capture *gct) {
  gct->id = CAPTURE_NUMBER_NOT_SET;
  gct->pcre_capture_number = CAPTURE_NUMBER_NOT_SET;

  gct->name = NULL;
  gct->subname = NULL;
  gct->pattern = NULL;
  gct->predicate_lib = NULL;
  gct->predicate_func_name = NULL;
  gct->extra.extra_len = 0;
  gct->extra.extra_val = NULL;
}

int grok_capture_add(grok_t *grok, grok_capture *gct) {
  DB *db = grok->captures_by_id;
  DBT key, value;
  int ret;

  grok_log(grok, LOG_CAPTURE, 
           "Adding pattern '%s' as capture %d (pcrenum %d)",
           gct->name, gct->id, gct->pcre_capture_number);

  /* Primary key is id */
  memset(&key, 0, sizeof(key));
  memset(&value, 0, sizeof(value));
  key.data = &(gct->id);
  key.size = sizeof(gct->id);

  _grok_capture_encode(gct, (char **)&value.data, &value.size);
  ret = db->put(db, NULL, &key, &value, 0);
  free(value.data);

  if (ret != 0) {
    db->err(db, ret, "grok_capture_add failed");
  }
  return ret;
}

int _grok_capture_get_db(grok_t *grok, DB *db, DBT *key, grok_capture *gct) {
  DBT value;
  int ret;

  memset(&value, 0, sizeof(DBT));

  ret = db->get(db, NULL, key, &value, 0);
  if (ret != 0) {
    return ret;
  }
  _grok_capture_decode(gct, (char *)value.data, value.size);
  return 0;
}

int grok_capture_get_by_id(grok_t *grok, int id, grok_capture *gct) {
  DBT key;
  int ret;
  memset(&key, 0, sizeof(DBT));
  key.data = &id;
  key.size = sizeof(id);
  ret = _grok_capture_get_db(grok, grok->captures_by_id, &key, gct);
  return ret;
}

int grok_capture_get_by_name(grok_t *grok, const char *name,
                              grok_capture *gct) {
  DBT key;
  int ret;
  memset(&key, 0, sizeof(DBT));
  key.data = (char *)name;
  key.size = strlen(name);
  ret = _grok_capture_get_db(grok, grok->captures_by_name, &key, gct);
  return ret;
}

int grok_capture_get_by_subname(grok_t *grok, const char *subname,
                                grok_capture *gct) {
  DBT key;
  int ret;
  memset(&key, 0, sizeof(DBT));
  key.data = (char *)subname;
  key.size = strlen(subname);
  ret = _grok_capture_get_db(grok, grok->captures_by_subname, &key, gct);
  return ret;
}

int grok_capture_get_by_capture_number(grok_t *grok, int capture_number,
                                        grok_capture *gct) {
  DBT key;
  int ret;
  memset(&key, 0, sizeof(DBT));
  key.data = &capture_number;
  key.size = sizeof(capture_number);
  ret = _grok_capture_get_db(grok, grok->captures_by_capture_number,
                             &key, gct);
  return ret;
}

int grok_capture_set_extra(grok_t *grok, grok_capture *gct, void *extra) {
  /* Store the pointer to extra.
   * XXX: This is potentially bad voodoo. */
  grok_log(grok, LOG_CAPTURE, "Setting extra value of 0x%x", extra);

  /* We could copy it this way, but if you compile with -fomit-frame-pointer,
   * this data is lost since extra is in the stack. Copy the pointer instead.
   */
  //gct->extra.extra_val = (char *)&extra;

  gct->extra.extra_len = sizeof(void *); /* allocate pointer size */
  gct->extra.extra_val = malloc(gct->extra.extra_len);
  memcpy(gct->extra.extra_val, &extra, gct->extra.extra_len);
  return 0;
}

void _grok_capture_encode(grok_capture *gct, char **data_ret,
                                    int *size_ret) {
  #define BASE_ALLOC_SIZE 256;
  XDR xdr;
  grok_capture local;
  int alloc_size = BASE_ALLOC_SIZE;
  *data_ret = NULL;

  /* xdr_string doesn't understand NULL, so replace NULL with "" */
  memcpy(&local, gct, sizeof(local));
  if (local.name == NULL) local.name = EMPTYSTR;
  if (local.subname == NULL) local.subname = EMPTYSTR;
  if (local.pattern == NULL) local.pattern = EMPTYSTR;
  if (local.predicate_lib == NULL) local.predicate_lib = EMPTYSTR;
  if (local.predicate_func_name == NULL) local.predicate_func_name = EMPTYSTR;
  if (local.extra.extra_val == NULL) local.extra.extra_val = EMPTYSTR;

  do {
    if (*data_ret == NULL) {
      *data_ret = malloc(alloc_size);
    } else {
      xdr_destroy(&xdr);
      alloc_size *= 2;
      //fprintf(stderr, "Growing xdr buffer to %d\n", alloc_size);
      *data_ret = realloc(*data_ret, alloc_size);
    }
    xdrmem_create(&xdr, *data_ret, alloc_size, XDR_ENCODE);

    /* If we get larger than a meg, something is probably wrong. */
    if (alloc_size > 1<<20) {
      abort();
    }
  } while (xdr_grok_capture(&xdr, &local) == FALSE);

  *size_ret = xdr_getpos(&xdr);
}

void _grok_capture_decode(grok_capture *gct, char *data, int size) {
  XDR xdr;

  xdrmem_create(&xdr, data, size, XDR_DECODE);
  xdr_grok_capture(&xdr, gct);
}

int _db_captures_by_name_key(DB *secondary, const DBT *key,
                                   const DBT *data, DBT *result) {
  grok_capture gct;
  int len;

  grok_capture_init(NULL, &gct);
  _grok_capture_decode(&gct, (char *)data->data, data->size);

  if (gct.name == NULL)
    return DB_DONOTINDEX;

  len = strlen(gct.name);

  result->data = malloc(len);
  memcpy(result->data, gct.name, len);
  result->size = len;

  result->flags |= DB_DBT_APPMALLOC;

  grok_capture_free(&gct);
  return 0;
}

int _db_captures_by_capture_number(DB *secondary, const DBT *key,
                                         const DBT *data, DBT *result) {
  grok_capture gct;
  grok_capture_init(NULL, &gct);
  _grok_capture_decode(&gct, (char *)data->data, data->size);

  if (gct.pcre_capture_number == CAPTURE_NUMBER_NOT_SET)
    return DB_DONOTINDEX;

  result->data = malloc(sizeof(int));
  *((int *)result->data) = gct.pcre_capture_number;
  result->size = sizeof(int);

  result->flags |= DB_DBT_APPMALLOC;

  grok_capture_free(&gct);
  return 0;
}

int _db_captures_by_subname(DB *secondary, const DBT *key,
                            const DBT *data, DBT *result) {
  grok_capture gct;
  grok_capture_init(NULL, &gct);
  _grok_capture_decode(&gct, (char *)data->data, data->size);

  if (gct.subname == NULL || *gct.subname == '\0')
    return DB_DONOTINDEX;

  result->size = strlen(gct.subname);
  result->data = malloc(result->size);
  memcpy(result->data, gct.subname, result->size);

  result->flags |= DB_DBT_APPMALLOC;

  grok_capture_free(&gct);
  return 0;
}

#define _GCT_STRFREE(gct, member) \
  if (gct->member != NULL && gct->member != EMPTYSTR) { \
    free(gct->member); \
  }

void grok_capture_free(grok_capture *gct) {
  _GCT_STRFREE(gct, name);
  _GCT_STRFREE(gct, subname);
  _GCT_STRFREE(gct, pattern);
  _GCT_STRFREE(gct, predicate_lib);
  _GCT_STRFREE(gct, predicate_func_name);
  _GCT_STRFREE(gct, extra.extra_val);
}

void *grok_capture_walk_init(grok_t *grok) {
  DBC *cursor;
  DB *db = grok->captures_by_id;
  db->cursor(db, NULL, &cursor, 0);

  DBT key; DBT value; int ret;
  memset(&key, 0, sizeof(DBT));
  memset(&value, 0, sizeof(DBT));
  while ((ret = cursor->c_get(cursor, &key, &value, DB_NEXT)) == 0) {
    grok_capture gct;
    grok_capture_init(grok, &gct);
    _grok_capture_decode(&gct, (char *)value.data, value.size);
    grok_capture_free(&gct);
  }

  cursor->c_close(cursor);

  grok->captures_by_name->cursor(grok->captures_by_name, NULL, &cursor, 0);
  return cursor;
}

int grok_capture_walk_next(grok_t *grok, void *handle, grok_capture *gct) {
  DBT key;
  DBT value;
  int ret;
  DBC *cursor = (DBC *)handle;

  memset(&key, 0, sizeof(DBT));
  memset(&value, 0, sizeof(DBT));
  //grok_log(grok, LOG_CAPTURE, "Fetching next from cursor");
  ret = cursor->c_get(cursor, &key, &value, DB_NEXT);

  if (ret != 0) {
    //grok->captures_by_id->err(grok->captures_by_id, ret, "cursor get error");
    return ret;
  }
  _grok_capture_decode(gct, (char *)value.data, value.size);
  return 0;
}

int grok_capture_walk_end(grok_t *grok, void *handle) {
  return ((DBC *)handle)->c_close((DBC *)handle);
}
