#include "grok.h"
#include "test.h"

void test_grok_pattern_add_and_find_work(void) {
  INIT;
  char *regexp;
  size_t len;

  grok_pattern_add(&grok, "WORD", 5, "\\w+", 3);
  grok_pattern_add(&grok, "TEST", 5, "TEST", 4);

  grok_pattern_find(&grok, "WORD", 5, &regexp, &len);
  CU_ASSERT(len == 3);
  CU_ASSERT(!strncmp(regexp, "\\w+", len));
  free(regexp);

  grok_pattern_find(&grok, "TEST", 5, &regexp, &len);
  CU_ASSERT(len == 4);
  CU_ASSERT(!strncmp(regexp, "TEST", len));
  free(regexp);

  CLEANUP;
}

void test_pattern_parse(void) {
  const char *name, *regexp;
  size_t name_len, regexp_len;

  _pattern_parse_string("WORD \\w+", &name, &name_len, &regexp, &regexp_len);
  CU_ASSERT(!strncmp(name, "WORD", name_len));
  CU_ASSERT(!strncmp(regexp, "\\w+", regexp_len));
  CU_ASSERT(name_len == 4);
  CU_ASSERT(regexp_len == 3);

  _pattern_parse_string("   NUM    numtest", &name, &name_len, &regexp, &regexp_len);
  CU_ASSERT(!strncmp(name, "NUM", name_len));
  CU_ASSERT(!strncmp(regexp, "numtest", regexp_len));
  CU_ASSERT(name_len == 3);
  CU_ASSERT(regexp_len == 7);

  _pattern_parse_string(" 	 NUMNUM 		 numtest",
                        &name, &name_len, &regexp, &regexp_len);
  CU_ASSERT(!strncmp(name, "NUMNUM", name_len));
  CU_ASSERT(!strncmp(regexp, "numtest", regexp_len));
  CU_ASSERT(name_len == 6);
  CU_ASSERT(regexp_len == 7);

}

void test_pattern_import_from_string(void) {
  INIT;
  char *buf = "WORD \\w+\n"
     "TEST test\n"
     "# This is a comment\n"
     "FOO bar\n";

  buf = strdup(buf);
  grok_patterns_import_from_string(&grok, buf);

  //CU_ASSERT(!strcmp(grok_pattern_find(&grok, "WORD", 5), "\\w+"));
  //CU_ASSERT(!strcmp(grok_pattern_find(&grok, "TEST", 5), "test"));
  //CU_ASSERT(!strcmp(grok_pattern_find(&grok, "FOO", 4), "bar"));

  free(buf);
  CLEANUP;
}
