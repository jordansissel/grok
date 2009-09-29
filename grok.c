#include "grok.h"
#include <dlfcn.h>

static int grok_pcre_callout(pcre_callout_block *pcb);

int g_grok_global_initialized = 0;
pcre *g_pattern_re = NULL;
int g_pattern_num_captures = 0;
int g_cap_name = 0;
int g_cap_pattern = 0;
int g_cap_subname = 0;
int g_cap_predicate = 0;

void grok_init(grok_t *grok) {
  //int ret;
  /* set global pcre_callout for libpcre */
  pcre_callout = grok_pcre_callout;

  grok->re = NULL;
  grok->full_pattern = NULL;
  grok->pcre_capture_vector = NULL;
  grok->pcre_num_captures = 0;
  grok->max_capture_num = 0;
  grok->pcre_errptr = NULL;
  grok->pcre_erroffset = 0;
  grok->logmask = 0;
  grok->logdepth = 0;

#ifndef GROK_TEST_NO_PATTERNS
  db_create(&grok->patterns, NULL, 0);
  grok->patterns->open(grok->patterns, NULL, NULL, "patterns",
                       DB_BTREE, DB_CREATE, 0);
#endif /* GROK_TEST_NO_PATTERNS */

#ifndef GROK_TEST_NO_CAPTURE
  db_create(&grok->captures_by_id, NULL, 0);
  db_create(&grok->captures_by_name, NULL, 0);
  db_create(&grok->captures_by_subname, NULL, 0);
  db_create(&grok->captures_by_capture_number, NULL, 0);

  /* Allow duplicates in _by_name */
  grok->captures_by_name->set_flags(grok->captures_by_name, DB_DUP);

  grok->captures_by_id->open(grok->captures_by_id, NULL, NULL,
                             "captures_by_id", DB_BTREE, DB_CREATE, 0);
  grok->captures_by_name->open(grok->captures_by_name, NULL, NULL,
                               "captures_by_name", DB_BTREE, DB_CREATE, 0);
  grok->captures_by_subname->open(grok->captures_by_subname, NULL, NULL,
                               "captures_by_subname", DB_BTREE, DB_CREATE, 0);
  grok->captures_by_capture_number->open(grok->captures_by_capture_number,
                                         NULL, NULL, "captures_by_capture_number",
                                         DB_BTREE, DB_CREATE, 0);

  /* Set up secondary index associations */
  grok->captures_by_id->associate(grok->captures_by_id, NULL, 
                                  grok->captures_by_name,
                                  _db_captures_by_name_key, 0);
  grok->captures_by_id->associate(grok->captures_by_id, NULL,
                                  grok->captures_by_capture_number,
                                  _db_captures_by_capture_number, 0);
  grok->captures_by_id->associate(grok->captures_by_id, NULL,
                                  grok->captures_by_subname,
                                  _db_captures_by_subname, 0);
#endif /* GROK_TEST_NO_CAPTURE */

  if (g_grok_global_initialized == 0) {
    /* do first initalization */
    g_grok_global_initialized = 1;

    /* VALGRIND NOTE: Valgrind complains here, but this is a global variable.
     * Ignore valgrind here. */
    g_pattern_re = pcre_compile(PATTERN_REGEX, 0,
                                &grok->pcre_errptr,
                                &grok->pcre_erroffset,
                                NULL);
    if (g_pattern_re == NULL) {
      fprintf(stderr, "Internal compiler error: %s\n", grok->pcre_errptr);
      fprintf(stderr, "Regexp: %s\n", PATTERN_REGEX);
      fprintf(stderr, "Position: %d\n", grok->pcre_erroffset);
    }

    pcre_fullinfo(g_pattern_re, NULL, PCRE_INFO_CAPTURECOUNT,
                  &g_pattern_num_captures);
    g_pattern_num_captures++; /* include the 0th group */
    g_cap_name = pcre_get_stringnumber(g_pattern_re, "name");
    g_cap_pattern = pcre_get_stringnumber(g_pattern_re, "pattern");
    g_cap_subname = pcre_get_stringnumber(g_pattern_re, "subname");
    g_cap_predicate = pcre_get_stringnumber(g_pattern_re, "predicate");
  }
}

void grok_clone(grok_t *dst, grok_t *src) {
  grok_init(dst);
  dst->patterns = src->patterns;
  dst->logmask = src->logmask;
  dst->logdepth = src->logdepth + 1;
}

static int grok_pcre_callout(pcre_callout_block *pcb) {
  grok_t *grok = pcb->callout_data;
  grok_capture gct;
  grok_capture_init(grok, &gct);

  //printf("callout: %d\n", pcb->capture_last);

  grok_capture_get_by_capture_number(grok, pcb->capture_last, &gct);

  if (gct.predicate_func_name != NULL) {
    int (*predicate)(grok_t *, grok_capture *, const char *, int, int);
    int start, end;
    void *handle;
    char *lib = gct.predicate_lib;
    start = pcb->offset_vector[ pcb->capture_last * 2 ];
    end = pcb->offset_vector[ pcb->capture_last * 2 + 1];

    /* XXX: call the predicate func */
    if (lib != NULL && lib[0] == '\0') {
      lib = NULL;
    }

    handle = dlopen(lib, RTLD_LAZY);
    predicate = dlsym(handle, gct.predicate_func_name);
    if (predicate != NULL) {
      return predicate(grok, &gct, pcb->subject, start, end);
    } else {
      grok_log(grok, LOG_EXEC, "No such function '%s' in library '%s'",
               gct.predicate_func_name, lib);
      return 0;
    }
  }
  return 0;
}
