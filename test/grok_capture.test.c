#include <string.h>
#include "grok.h"
#include "test.h"
#include "grok_capture.h"

void test_grok_capture_get_by_id(void) {
  INIT;

  grok_capture src;
  const grok_capture *dst;
  grok_capture_init(&grok, &src);

  src.id = 9;
  src.name = "Test";
  src.pcre_capture_number = 15;
  grok_capture_add(&grok, &src);
  dst = grok_capture_get_by_id(&grok, src.id);

  CU_ASSERT(src.id == dst->id);
  CU_ASSERT(src.pcre_capture_number == dst->pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst->name));
  CLEANUP;
} 

void test_grok_capture_get_by_name(void) {
  INIT;
  int ret;

  grok_capture src;
  const grok_capture *dst;
  grok_capture_init(&grok, &src);

  src.id = 9;
  src.name = "Test";
  src.pcre_capture_number = 15;
  grok_capture_add(&grok, &src);
  dst = grok_capture_get_by_name(&grok, src.name);
  CU_ASSERT(dst != NULL);
  CU_ASSERT(src.id == dst->id);
  CU_ASSERT(src.pcre_capture_number == dst->pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst->name));
  CLEANUP;
} 

void test_grok_capture_get_by_capture_number(void) {
  INIT;

  grok_capture src;
  const grok_capture *dst;
  grok_capture_init(&grok, &src);

  src.id = 0;
  src.name = strdup("Test");
  src.pcre_capture_number = 15;
  grok_capture_add(&grok, &src);
  dst = grok_capture_get_by_capture_number(&grok, src.pcre_capture_number);

  CU_ASSERT(dst != NULL);
  CU_ASSERT(src.id == dst->id);
  CU_ASSERT(src.pcre_capture_number == dst->pcre_capture_number);
  CU_ASSERT(!strcmp(src.name, dst->name));
  free(src.name);
}
