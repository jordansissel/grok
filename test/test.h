#define ASSERT_MATCHFAIL(str) \
  CU_ASSERT(grok_exec(&grok, str, NULL) == GROK_ERROR_NOMATCH)
#define ASSERT_MATCHOK(str) \
  CU_ASSERT(grok_exec(&grok, str, NULL) == GROK_OK)
#define ASSERT_COMPILEOK(str) \
  CU_ASSERT(grok_compile(&grok, str) == GROK_OK) 
#define ASSERT_COMPILEFAIL(str) \
  CU_ASSERT(grok_compile(&grok, str) != GROK_OK) 

#ifndef LOG_MASK
#define LOG_MASK 0
#endif

/* Helpers */

#define INIT \
  grok_t grok; \
  grok_init(&grok); \
  grok.logmask = LOG_MASK;

#define CLEANUP \
  grok_free(&grok);

#define IMPORT_PATTERNS_FILE grok_patterns_import_from_file(&grok, "../patterns/base")
