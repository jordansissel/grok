#ifndef _GROK_H_
#define _GROK_H_

#include <pcre.h>
#include <tcutil.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <db.h>
#include <string.h>

typedef struct grok grok_t;

typedef struct grok_pattern {
  const char *name;
  char *regexp;
} grok_pattern_t;

struct grok {
  //DB *patterns;
  TCTREE *patterns;
  
  /* These are initialized when grok_compile is called */
  pcre *re;
  const char *pattern;
  int pattern_len;
  char *full_pattern;
  int full_pattern_len;
  int *pcre_capture_vector;
  int pcre_num_captures;
  
  /* Data storage for named-capture (grok capture) information */
  //TCTREE *captures_by_id;
  TCTREE *captures_by_id;
  TCTREE *captures_by_name;
  TCTREE *captures_by_subname;
  TCTREE *captures_by_capture_number;
  int max_capture_num;
  
  /* PCRE pattern compilation errors */
  const char *pcre_errptr;
  int pcre_erroffset;
  int pcre_errno;

  unsigned int logmask;
  unsigned int logdepth;
  char *errstr;
};

extern int g_grok_global_initialized;
extern pcre *g_pattern_re;
extern int g_pattern_num_captures;
extern int g_cap_name;
extern int g_cap_pattern;
extern int g_cap_subname;
extern int g_cap_predicate;

/* pattern to match %{FOO:BAR} */
/* or %{FOO<=3} */

#define PATTERN_REGEX \
  "(?!<\\\\)%{" \
  "(?<name>" \
    "(?<pattern>[A-z0-9]+)" \
    "(?::(?<subname>[A-z0-9]+))?" \
  ")" \
  "\\s*(?<predicate>" \
    "(?:" \
      "(?P<curly>\\{(?:(?>[^{}]+|(?>\\\\[{}])+)|(?P>curly))*\\})" \
      "|" \
      "(?:[^{}]+|\\\\[{}])+" \
    ")+" \
  ")?" \
  "}"

#define GROK_OK 0
#define GROK_ERROR_FILE_NOT_ACCESSIBLE 1
#define GROK_ERROR_PATTERN_NOT_FOUND 2
#define GROK_ERROR_UNEXPECTED_READ_SIZE 3
#define GROK_ERROR_COMPILE_FAILED 4
#define GROK_ERROR_UNINITIALIZED 5
#define GROK_ERROR_PCRE_ERROR 6
#define GROK_ERROR_NOMATCH 7

#define CAPTURE_ID_LEN 4
#define CAPTURE_FORMAT "%04x"

#include "grok_logging.h"

#ifndef GROK_TEST_NO_PATTERNS
#include "grok_pattern.h"
#endif

#ifndef GROK_TEST_NO_CAPTURE
#include "grok_capture.h"
#endif

#include "grok_match.h"
#include "grokre.h"
#include "grok_discover.h"

#endif /* ifndef _GROK_H_ */
