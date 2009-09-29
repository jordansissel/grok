#include <string.h>
#include "grok.h"
#include "test.h"
#include "grok_capture.h"

void test_grok_capture_encode_and_decode(void) {
  INIT;

  char *data;
  int len;

  grok_capture src, dst;

  /* This is critical, otherwise dst.name might be non-null which breaks
   * xdr_string  assuming that non-null means allocated and access yields a
   * segfault. */
  memset(&src, 0, sizeof(grok_capture));
  memset(&dst, 0, sizeof(grok_capture));

  src.id = 5;
  src.pcre_capture_number = 20;
  src.name = "Testing";
  src.pattern = "FOO";
  src.predicate_lib = "";
  src.predicate_func_name = "myfunc";

  _grok_capture_encode(&src, &data, &len);
  _grok_capture_decode(&dst, data, len);

  CU_ASSERT(src.id == dst.id);
  CU_ASSERT(src.pcre_capture_number == dst.pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst.name));
  CU_ASSERT(!strcmp(src.pattern, dst.pattern));
  CU_ASSERT(!strcmp(src.predicate_func_name, dst.predicate_func_name));

  free(data);
  grok_capture_free(&dst);
  
  CLEANUP;
}

void test_grok_capture_encode_and_decode_large(void) {
  INIT;

  char *data;
  int len;

  grok_capture src, dst;

  grok_capture_init(&grok, &src);
  grok_capture_init(&grok, &dst);

  src.id = 5;
  src.pcre_capture_number = 20;
  src.name = "Testing";
  src.pattern = "this is some long string that should make the encoding longer than the base allocation size";
  src.predicate_func_name = "myfunc";

  _grok_capture_encode(&src, &data, &len);
  _grok_capture_decode(&dst, data, len);

  /* len should be more than BASE_ALLOC_SIZE (grok_capture.c) */
  CU_ASSERT(len > 64);

  CU_ASSERT(src.id == dst.id);
  CU_ASSERT(src.pcre_capture_number == dst.pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst.name));
  CU_ASSERT(!strcmp(src.pattern, dst.pattern));
  CU_ASSERT(!strcmp(src.predicate_func_name, dst.predicate_func_name));

  grok_capture_free(&dst);
  free(data);
  CLEANUP;
}

void test_grok_capture_get_db(void) {
  INIT;
  DBT key;
  int e;
  memset(&key, 0, sizeof(key));

  grok_capture src, dst;
  grok_capture_init(&grok, &src);
  grok_capture_init(&grok, &dst);

  src.id = 9;
  src.name = "Test";
  src.pcre_capture_number = 15;
  e = grok_capture_add(&grok, &src);

  memset(&key, 0, sizeof(DBT));
  key.data = &src.id;
  key.size = sizeof(src.id);

  _grok_capture_get_db(&grok, grok.captures_by_id, &key, &dst);
  CU_ASSERT(src.id == dst.id);
  CU_ASSERT(src.pcre_capture_number == dst.pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst.name));
  CLEANUP;
}

void test_grok_capture_get_by_id(void) {
  INIT;

  grok_capture src, dst;
  grok_capture_init(&grok, &src);
  grok_capture_init(&grok, &dst);

  src.id = 9;
  src.name = "Test";
  src.pcre_capture_number = 15;
  grok_capture_add(&grok, &src);
  grok_capture_get_by_id(&grok, src.id, &dst);

  CU_ASSERT(src.id == dst.id);
  CU_ASSERT(src.pcre_capture_number == dst.pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst.name));
  CLEANUP;
} 

void test_grok_capture_get_by_name(void) {
  INIT;
  int ret;

  grok_capture src, dst;
  grok_capture_init(&grok, &src);
  grok_capture_init(&grok, &dst);

  src.id = 9;
  src.name = "Test";
  src.pcre_capture_number = 15;
  grok_capture_add(&grok, &src);
  ret = grok_capture_get_by_name(&grok, src.name, &dst);
  if (ret != 0) {
    DB *db = grok.captures_by_name;
    db->err(db, ret, "get_by_name");
  }

  CU_ASSERT(src.id == dst.id);
  CU_ASSERT(src.pcre_capture_number == dst.pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst.name));
  CLEANUP;
} 

void test_grok_capture_get_by_capture_number(void) {
  INIT;

  grok_capture src, dst;
  grok_capture_init(&grok, &src);
  memset(&dst, 0, sizeof(dst));

  src.id = 0;
  src.name = strdup("Test");
  src.pcre_capture_number = 15;
  grok_capture_add(&grok, &src);
  grok_capture_get_by_capture_number(&grok, src.pcre_capture_number, &dst);

  CU_ASSERT(src.id == dst.id);
  CU_ASSERT(src.pcre_capture_number == dst.pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst.name));
  free(src.name);
}
