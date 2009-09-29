#include "grok.h"
#include "test.h"

void test_grok_pcre_compile_succeeds(void) {
  INIT;

  ASSERT_COMPILEOK("\\w+");
  ASSERT_COMPILEOK("");
  ASSERT_COMPILEOK("testing");
  ASSERT_COMPILEFAIL("[");
  ASSERT_COMPILEFAIL("[a");

  CLEANUP;
}

void test_grok_pcre_match(void) {
  INIT;

  ASSERT_COMPILEOK("[a-z]+");
  ASSERT_MATCHOK("foo");
  ASSERT_MATCHOK("  one two  ");
  ASSERT_MATCHFAIL("...");
  ASSERT_MATCHFAIL("1234");

  CLEANUP;
}

void test_grok_match_with_patterns(void) {
  INIT;

  grok_patterns_import_from_string(&grok, "WORD \\b\\w+\\b");

  ASSERT_COMPILEOK("%{WORD}");

  ASSERT_MATCHOK("testing");
  ASSERT_MATCHOK("  one two  ");
  ASSERT_MATCHFAIL("---");
  ASSERT_MATCHFAIL("-.");

  CLEANUP;
}

void test_grok_match_with_escaped_pattern(void) {
  INIT;

  grok_patterns_import_from_string(&grok, "WORD \\b\\w+\\b");

  ASSERT_COMPILEOK("\\%\\{WORD\\}");

  ASSERT_MATCHOK("%{WORD}");
  ASSERT_MATCHFAIL("testing");
  ASSERT_MATCHFAIL("another test");

  ASSERT_COMPILEOK("\\%\\{%{WORD}\\}");
  ASSERT_MATCHOK("%{WORD}");
  ASSERT_MATCHOK("%{TESTING}");
  ASSERT_MATCHOK("%{FIZZ}");
  CLEANUP;
}

void test_grok_match_substr(void) {
  INIT;
  grok_match_t gm;
  
  ASSERT_COMPILEOK("\\w+ world");

  CU_ASSERT(grok_exec(&grok, "something hello world", &gm) >= 0);
  CU_ASSERT(grok.pcre_capture_vector[0] == 10); // start of match
  CU_ASSERT(grok.pcre_capture_vector[1] == 21); // end of match

  // XXX: make function:
  // int grok_match_string(grok_t, grok_match_t, char **matchstr, int *matchlen)
  // verify the matched string is 'hello world'
  CU_ASSERT(!strncmp("hello world",
                     gm.subject + grok.pcre_capture_vector[0],
                     grok.pcre_capture_vector[1] - grok.pcre_capture_vector[0]));
  CLEANUP;
}

void test_grok_match_get_named_substring(void) {
  INIT;
  IMPORT_PATTERNS_FILE;  
  grok_match_t gm;
  const char *str;
  int len;

  ASSERT_COMPILEOK("hello %{WORD}");
  ASSERT_MATCHOK("hello world");

  CU_ASSERT(grok_exec(&grok, "hello world", &gm) == GROK_OK);
  grok_match_get_named_substring(&gm, "WORD", &str, &len);

  //CU_ASSERT(len == 5);
  //CU_ASSERT(!strncmp(str, "world", len));

  CLEANUP;
}
