#include "grok.h"
#include "test.h"

void test_grok_with_predicate_compile_succeeds(void) {
  INIT;
  grok_patterns_import_from_string(&grok, "WORD \\b\\w+\\b");

  ASSERT_COMPILEOK("%{WORD=~/test/}");
  ASSERT_COMPILEOK("%{WORD!~/test/}");
  ASSERT_COMPILEOK("%{WORD>30}");
  ASSERT_COMPILEOK("%{WORD>=30}");
  ASSERT_COMPILEOK("%{WORD<30}");
  ASSERT_COMPILEOK("%{WORD<=30}");
  ASSERT_COMPILEOK("%{WORD==30}");
  ASSERT_COMPILEOK("%{WORD!=30}");

  CLEANUP;
}

void test_grok_with_numcompare_gt_normal(void) {
  INIT;
  IMPORT_PATTERNS_FILE;

  (void) grok_compile(&grok, "^%{NUMBER>10}$");

  ASSERT_MATCHFAIL("0");
  ASSERT_MATCHFAIL("1");
  ASSERT_MATCHFAIL("9");
  ASSERT_MATCHFAIL("10");
  ASSERT_MATCHFAIL("5.5");
  ASSERT_MATCHFAIL("0.2");
  ASSERT_MATCHFAIL("9.95");

  /* Should fail since '10' means we use a long, not a double */
  ASSERT_MATCHFAIL("10.1")
  ASSERT_MATCHFAIL("10.2")

  ASSERT_MATCHOK("11.2")
  ASSERT_MATCHOK("4425.334")
  ASSERT_MATCHOK("11");
  ASSERT_MATCHOK("15");

  CLEANUP;
}

void test_grok_with_numcompare_gt_double(void) {
  INIT;
  IMPORT_PATTERNS_FILE;

  (void) grok_compile(&grok, "^%{NUMBER>10.0}$");

  ASSERT_MATCHFAIL("0");
  ASSERT_MATCHFAIL("1");
  ASSERT_MATCHFAIL("9");
  ASSERT_MATCHFAIL("10");
  ASSERT_MATCHFAIL("5.5");
  ASSERT_MATCHFAIL("0.2");
  ASSERT_MATCHFAIL("9.95");

  /* Should pass since '10.0' means we use a double */
  ASSERT_MATCHOK("10.1")
  ASSERT_MATCHOK("10.2")

  ASSERT_MATCHOK("11.2")
  ASSERT_MATCHOK("4425.334")
  ASSERT_MATCHOK("11");
  ASSERT_MATCHOK("15");

  CLEANUP;
}

void test_grok_with_numcompare_gt_hex(void) {
  INIT;
  IMPORT_PATTERNS_FILE;

  grok_compile(&grok, "%{BASE16FLOAT>0x000A}");

  ASSERT_MATCHFAIL("0");
  ASSERT_MATCHFAIL("1");
  ASSERT_MATCHFAIL("9");
  ASSERT_MATCHFAIL("10");
  ASSERT_MATCHFAIL("0x05");
  ASSERT_MATCHFAIL("0x0A");

  ASSERT_MATCHOK("11");
  ASSERT_MATCHOK("15");
  ASSERT_MATCHOK("0x0B");
  ASSERT_MATCHOK("0xB");
  ASSERT_MATCHOK("0xFF");

  CLEANUP;
}

void test_grok_numcompare_lt(void) {
  INIT;
  IMPORT_PATTERNS_FILE;

  ASSERT_COMPILEOK("%{NUMBER<57}");
  ASSERT_MATCHOK("-13");
  ASSERT_MATCHOK("-3");
  ASSERT_MATCHOK("0");
  ASSERT_MATCHOK("3");
  ASSERT_MATCHOK("13");
  ASSERT_MATCHOK("56");
  ASSERT_MATCHFAIL("57");
  ASSERT_MATCHFAIL("58");
  ASSERT_MATCHFAIL("70");
  ASSERT_MATCHFAIL("100");
  ASSERT_MATCHFAIL("5825");

  CLEANUP;
}

void test_grok_numcompare_le(void) {
  INIT;
  IMPORT_PATTERNS_FILE;

  ASSERT_COMPILEOK("%{NUMBER<=57}");
  ASSERT_MATCHOK("-13");
  ASSERT_MATCHOK("-3");
  ASSERT_MATCHOK("0");
  ASSERT_MATCHOK("3");
  ASSERT_MATCHOK("13");
  ASSERT_MATCHOK("56");
  ASSERT_MATCHOK("57");
  ASSERT_MATCHFAIL("58");
  ASSERT_MATCHFAIL("70");
  ASSERT_MATCHFAIL("100");
  ASSERT_MATCHFAIL("5825");

  CLEANUP;
}
