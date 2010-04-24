/**
 * @file grok.h
 */
#ifndef _GROK_H_
#define _GROK_H_

#include <pcre.h>
#include <tcutil.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * A regular expression super library.
 */

typedef struct grok grok_t;

typedef struct grok_pattern {
  const char *name;
  char *regexp;
} grok_pattern_t;

struct grok {
  /** The original pattern given to grok_compile() */
  const char *pattern;

  /** length of the pattern string */
  int pattern_len;

  /** The full expanded pattern generated from grok_compile() */
  char *full_pattern;

  /** full_pattern string length */
  int full_pattern_len;

  /** tokyocabinet TCTREE of patterns */
  TCTREE *patterns;

  pcre *re;
  int *pcre_capture_vector;
  int pcre_num_captures;
  
  /* Data storage for named-capture (grok capture) information */
  TCTREE *captures_by_id;
  TCTREE *captures_by_name;
  TCTREE *captures_by_subname;
  TCTREE *captures_by_capture_number;
  int max_capture_num;
  
  /** PCRE pattern compilation errors */
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

/** Safe return code */
#define GROK_OK 0

/** File not accessible. This occurs when you try to load patterns from
 * a file and grok cannot read it. */
#define GROK_ERROR_FILE_NOT_ACCESSIBLE 1

/** Pattern not found is given if you try to search for a pattern
 * by name that is not known, that is, it wasn't added with
 * grok_pattern_add or grok_patterns_import* */
#define GROK_ERROR_PATTERN_NOT_FOUND 2

#define GROK_ERROR_UNEXPECTED_READ_SIZE 3

/** grok_compile failed. */
#define GROK_ERROR_COMPILE_FAILED 4

/** grok_exec called on a grok_t instance that was not initialized first.
 * Make sure you grok_init(). */
#define GROK_ERROR_UNINITIALIZED 5

/** A PCRE-related error occurred. Check the grok_t pcre_errno and other
 * pcre_err* members */
#define GROK_ERROR_PCRE_ERROR 6

/** grok_exec did not match your string */
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
#include "grok_discover.h"
#include "grok_version.h"

/**
 * @mainpage
 *
 * Test foo bar
 *
 * baz fizz
 *
 * grok_new()
 */

/**
 * Create a new grok_t instance.
 *
 * The new grok_t instance is initialized with grok_init() already,
 * so you do not need to call it.
 *
 * You should use grok_free() on the result when you want to free it.
 *
 * @ingroup grok_t
 * @return pointer to new grok_t instance or NULL on failure.
 */
grok_t *grok_new();

/**
 * Initialize a grok_t instance. This is useful if you have
 * a grok_t as a stack variable rather than heap.
 *
 * @ingroup grok_t
 */
void grok_init(grok_t *grok);

/**
 * Shallow clone of a grok instance.
 * This is useful for creating new grok_t instances with the same
 * loaded patterns. It also copies the log settings.
 *
 * @param dst pointer to destination grok_t you want to clone into.
 * @param src pointer to source grok_t you can to clone from.
 */
void grok_clone(grok_t *dst, const grok_t *src);

/**
 * Free a grok instance. This will clean up any memory allocated by the 
 * grok_t instance during it's life. Finally, this method will free()
 * the grok_t pointer you gave.
 *
 * Do not use this method on grok_t instances you created with
 * grok_clone.
 *
 * @param grok the grok_t instance you want to free.
 */
void grok_free(grok_t *grok);

/**
 */
void grok_free_clone(const grok_t *grok);

/**
 * Get the library's version number
 *
 * @return string representing grok's version.
 */
const char *grok_version();

/**
 * Compile a pattern. PCRE syntax is supported.
 *
 * Grok extends this syntax with %{PATTERN} syntax
 * This allows you to use predefined patterns anywhere in your
 * regular expression.
 *
 * @param grok the grok_t instance to compile into.
 * @param pattern the string regexp pattern to compile.
 */
int grok_compile(grok_t *grok, const char *pattern);

/**
 * Compile a pattern with known length.
 *
 * @param pattern the string pattern to compile
 * @param length the length of the pattern
 * @see grok_compile()
 */
int grok_compilen(grok_t *grok, const char *pattern, int length);

/**
 * Execute against a string input.
 *
 * @param text the text to match.
 * @param gm The grok_match_t to store match result in.  If NULL, no storing is attempted.
 * @returns GROK_OK if match successful, GROK_ERROR_NOMATCH if no match.
 */
int grok_exec(const grok_t *grok, const char *text, grok_match_t *gm);

/**
 * @see grok_exec
 * */
int grok_execn(const grok_t *grok, const char *text, int textlen, grok_match_t *gm);

int grok_match_get_named_substring(const grok_match_t *gm, const char *name,
                                   const char **substr, int *len);


#endif /* ifndef _GROK_H_ */
