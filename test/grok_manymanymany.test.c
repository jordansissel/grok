#include "grok.h"
#include "test.h"
#include <string.h>

void test_manymanymany(void) {
  INIT;
  IMPORT_PATTERNS_FILE;

  int i;
  grok_match_t gm;
  const char *str;
  int len;
  ASSERT_COMPILEOK("%{NUMBER}");

  for (i = -10000; i < 10000; i++) {
    char buf[30];
    sprintf(buf, "%d", i);
    CU_ASSERT(grok_exec(&grok, buf, &gm) == GROK_OK);
    grok_match_get_named_substring(&gm, "NUMBER", &str, &len);
    CU_ASSERT(strncmp(str, buf, len) == 0);
  }

  CLEANUP;
} 
