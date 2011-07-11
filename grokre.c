#include <pcre.h>
#include <assert.h>
#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
#include <search.h>
#include <ctype.h>

#include "grok.h"
#include "predicates.h"
#include "stringhelper.h"

/* global, static variables */

#define CAPTURE_ID_LEN 4
#define CAPTURE_FORMAT "%04x"

/* internal functions */
static char *grok_pattern_expand(grok_t *grok); //, int offset, int length);
static void grok_study_capture_map(grok_t *grok);

static void grok_capture_add_predicate(grok_t *grok, int capture_id,
                                       const char *predicate, int predicate_len);

void grok_free_clone(const grok_t *grok) {
  if (grok->re != NULL) {
    pcre_free(grok->re);
  }

  if (grok->full_pattern != NULL) {
    free(grok->full_pattern);
  }

  if (grok->pcre_capture_vector != NULL) {
    free(grok->pcre_capture_vector);
  }

  if (grok->captures_by_name != NULL) {
    tctreedel(grok->captures_by_name);
  }

  if (grok->captures_by_subname != NULL) {
    tctreedel(grok->captures_by_subname);
  }

  if (grok->captures_by_capture_number != NULL) {
    tctreedel(grok->captures_by_capture_number);
  }

  if (grok->captures_by_id != NULL) {
    tctreedel(grok->captures_by_id);
  }
}

void grok_free(grok_t *grok) {
  grok_free_clone(grok);
  if (grok->patterns != NULL)
    tctreedel(grok->patterns);
}

int grok_compile(grok_t *grok, const char *pattern) {
  return grok_compilen(grok, pattern, strlen(pattern));
}

int grok_compilen(grok_t *grok, const char *pattern, int length) {
  grok_log(grok, LOG_COMPILE, "Compiling '%.*s'", length, pattern);

  /* clear the old tctree data */
  tctreeclear(grok->captures_by_name);
  tctreeclear(grok->captures_by_subname);
  tctreeclear(grok->captures_by_capture_number);
  tctreeclear(grok->captures_by_id);

  grok->pattern = pattern;
  grok->pattern_len = length;
  grok->full_pattern = grok_pattern_expand(grok);

  if (grok->full_pattern == NULL) {
    grok_log(grok, LOG_COMPILE, "A failure occurred while compiling '%.*s'",
             length, pattern);
    grok->errstr = "failure occurred while expanding pattern "\
                   "(too pattern recursion?)";
    return GROK_ERROR_COMPILE_FAILED;
  }

  grok->re = pcre_compile(grok->full_pattern, 0, 
                          &grok->pcre_errptr, &grok->pcre_erroffset, NULL);

  if (grok->re == NULL) {
    grok->errstr = (char *)grok->pcre_errptr;
    return GROK_ERROR_COMPILE_FAILED;
  }

  pcre_fullinfo(grok->re, NULL, PCRE_INFO_CAPTURECOUNT, &grok->pcre_num_captures);
  grok->pcre_num_captures++; /* include the 0th group */
  grok->pcre_capture_vector = calloc(3 * grok->pcre_num_captures, sizeof(int));

  /* Walk grok->captures_by_id.
   * For each, ask grok->re what stringnum it is */
  grok_study_capture_map(grok);

  return GROK_OK;
}

const char * const grok_error(grok_t *grok) {
  return grok->errstr;
}

int grok_exec(const grok_t *grok, const char *text, grok_match_t *gm) {
  return grok_execn(grok, text, strlen(text), gm);
}

int grok_execn(const grok_t *grok, const char *text, int textlen, grok_match_t *gm) {
  int ret;
  pcre_extra pce;
  pce.flags = PCRE_EXTRA_CALLOUT_DATA;
  pce.callout_data = (void *)grok;

  if (grok->re == NULL) {
    grok_log(grok, LOG_EXEC, "Error: pcre re is null, meaning you haven't called grok_compile yet");
    fprintf(stderr, "ERROR: grok_execn called on an object that has not pattern compiled. Did you call grok_compile yet?\n");
    return GROK_ERROR_UNINITIALIZED;
  }

  ret = pcre_exec(grok->re, &pce, text, textlen, 0, 0,
                  grok->pcre_capture_vector, grok->pcre_num_captures * 3);
  grok_log(grok, LOG_EXEC, "%.*s =~ /%s/ => %d",
           textlen, text, grok->pattern, ret);
  if (ret < 0) {
    switch (ret) {
      case PCRE_ERROR_NOMATCH:
        return GROK_ERROR_NOMATCH;
        break;
      case PCRE_ERROR_NULL:
        fprintf(stderr, "Null error, one of the arguments was null?\n");
        break;
      case PCRE_ERROR_BADOPTION:
        fprintf(stderr, "pcre badoption\n");
        break;
      case PCRE_ERROR_BADMAGIC:
        fprintf(stderr, "pcre badmagic\n");
        break;
    }
    //grok->pcre_errno = ret;
    return GROK_ERROR_PCRE_ERROR;
  }

  /* Push match info into gm only if it is non-NULL */
  if (gm != NULL) {
    gm->grok = grok;
    gm->subject = text;
    gm->start = grok->pcre_capture_vector[0];
    gm->end = grok->pcre_capture_vector[1];
  }

  return GROK_OK;
}

/* XXX: This method is pretty long; split it up? */
static char *grok_pattern_expand(grok_t *grok) {
  int capture_id = 0; /* Starting capture_id, doesn't really matter what this is */
  int offset = 0; /* string offset; how far we've expanded so far */
  int *capture_vector = NULL;
  int replacement_count = 0; /* count of replacements of %{foo} with a regexp */

  int full_len = -1;
  int full_size = -1;
  char *full_pattern = NULL;
  char capture_id_str[CAPTURE_ID_LEN + 1];

  const char *patname = NULL;

  capture_vector = calloc(3 * g_pattern_num_captures, sizeof(int));
  full_len = grok->pattern_len;
  full_size = full_len;
  full_pattern = calloc(1, full_size);
  memcpy(full_pattern, grok->pattern, full_len);
  grok_log(grok, LOG_REGEXPAND, "% 20s: %.*s", "start of expand",
           full_len, full_pattern);

  while (pcre_exec(g_pattern_re, NULL, full_pattern, full_len, offset, 
                   0, capture_vector, g_pattern_num_captures * 3) >= 0) {
    int start, end, matchlen;
    const char *pattern_regex;
    int patname_len;
    size_t regexp_len;
    int pattern_regex_needs_free = 0;

    grok_log(grok, LOG_REGEXPAND, "% 20s: %.*s", "start of loop",
             full_len, full_pattern);

    replacement_count++;
    if (replacement_count > 500) {
      free(capture_vector);
      free(full_pattern);
      grok->errstr = "Too many replacements have occurred (500), infinite recursion?";
      return NULL;
    }

    start = capture_vector[0];
    end = capture_vector[1];
    matchlen = end - start;
    grok_log(grok, LOG_REGEXPAND, "Pattern length: %d", matchlen);

    pcre_get_substring(full_pattern, capture_vector, g_pattern_num_captures,
                       g_cap_pattern, &patname);
    patname_len = capture_vector[g_cap_pattern * 2 + 1] \
                  - capture_vector[g_cap_pattern * 2];
    grok_log(grok, LOG_REGEXPAND, "Pattern name: %.*s", patname_len, patname);

    grok_pattern_find(grok, patname, patname_len, &pattern_regex, &regexp_len);

    if (pattern_regex == NULL) {
      /* Pattern not found, check if this has an in-line definition */
      int definition_len = capture_vector[g_cap_definition * 2 + 1] 
                           - capture_vector[g_cap_definition * 2];
      if (definition_len > 0) {
        /* We got an in-line definition */
        /* discard const-ness with reckless abandon! */
        pattern_regex = string_ndup(full_pattern + capture_vector[g_cap_definition * 2], definition_len);
        regexp_len = definition_len;
        pattern_regex_needs_free = 1;
        grok_log(grok, LOG_REGEXPAND, "Inline-definition found: %.*s => '%.*s'",
                 patname_len, patname, regexp_len, pattern_regex);
      } else {
        /* If we get here, there's definitely no pattern available for the 
         * current %{thing}, so just leave it unmolested. */
        offset = end;
      }
    }
    
    /* Check for nullness again because there could've been an inline
     * definition found above */
    if (pattern_regex != NULL) {
      int has_predicate = (capture_vector[g_cap_predicate * 2] >= 0);
      const char *longname = NULL;
      const char *subname = NULL;
      grok_capture *gct = calloc(1, sizeof(grok_capture));;

      /* XXX: Change this to not use pcre_get_substring so we can skip a
       * malloc step? */
      pcre_get_substring(full_pattern, capture_vector, g_pattern_num_captures,
                         g_cap_name, &longname);
      pcre_get_substring(full_pattern, capture_vector, g_pattern_num_captures,
                         g_cap_subname, &subname);

      snprintf(capture_id_str, CAPTURE_ID_LEN + 1, CAPTURE_FORMAT, capture_id);

      /* Add this capture to the list of captures */
      gct->id = capture_id;
      gct->name = (char *)longname; /* XXX: CONST PROBLEM */
      gct->name_len = strlen(gct->name);
      gct->subname = (char *)subname;
      gct->subname_len = strlen(gct->subname);
      grok_capture_add(grok, gct);

      //pcre_free_substring(longname);
      //pcre_free_substring(subname);

      /* if a predicate was given, add (?C1) to callout when the match is made,
       * so we can test it further */
      if (has_predicate) {
        int pstart, pend;
        pstart = capture_vector[g_cap_predicate * 2];
        pend = capture_vector[g_cap_predicate * 2 + 1];
        grok_log(grok, LOG_REGEXPAND, "Predicate found in '%.*s'",
                 matchlen, full_pattern + start);
        grok_log(grok, LOG_REGEXPAND, "Predicate is: '%.*s'",
                 pend - pstart, full_pattern + pstart);

        grok_capture_add_predicate(grok, capture_id, full_pattern + pstart,
                                   pend - pstart);
        substr_replace(&full_pattern, &full_len, &full_size,
                       end, 0, "(?C1)", 5);
      }

      /* Replace %{FOO} with (?<>). '5' is strlen("(?<>)") */
      substr_replace(&full_pattern, &full_len, &full_size,
                     start, end, "(?<>)", 5);
      grok_log(grok, LOG_REGEXPAND, "% 20s: %.*s", "replace with (?<>)",
               full_len, full_pattern);

      /* Insert the capture id into (?<FOO>) */
      substr_replace(&full_pattern, &full_len, &full_size,
                     start + 3, 0,
                     capture_id_str, CAPTURE_ID_LEN);
      grok_log(grok, LOG_REGEXPAND, "% 20s: %.*s", "add capture id",
               full_len, full_pattern);


      /* Insert the pattern into (?<FOO>pattern) */
      /* 3 = '(?<', 4 = strlen(capture_id_str), 1 = ")" */
      substr_replace(&full_pattern, &full_len, &full_size, 
                     start + 3 + CAPTURE_ID_LEN + 1, 0, 
                     pattern_regex, regexp_len);
      grok_log(grok, LOG_REGEXPAND, ":: Inserted: %.*s", regexp_len,
               pattern_regex);
      grok_log(grok, LOG_REGEXPAND, ":: STR: %.*s", full_len, full_pattern);

      /* Invariant, full_pattern actual len must always be full_len */
      assert(strlen(full_pattern) == full_len);
      
      /* Move offset to the start of the regexp pattern we just injected.
       * This is so when we iterate again, we can process this new pattern
       * to see if the regexp included itself any %{FOO} things */
      offset = start;
      capture_id++;
    }

    if (pattern_regex_needs_free) {
      /* If we need to free,   */
      free(pattern_regex);
    }
    if (patname != NULL) {
      pcre_free_substring(patname);
      patname = NULL;
    }
  } /* while pcre_exec */

  /* Unescape any "\%" strings found */
  offset = 0;
  while (offset < full_len) { /* loop to '< full_len' because we access offset+1 */
    if (full_pattern[offset] == '\\' && full_pattern[offset + 1] == '%') {
      substr_replace(&full_pattern, &full_len, &full_size,
                     offset, offset + 1, "", 0);
    }
    offset++;
  }

  grok_log(grok, LOG_REGEXPAND, "Fully expanded: %.*s", full_len, full_pattern);

  free(capture_vector);
  grok->full_pattern_len = full_len;
  grok->full_pattern = full_pattern;
  return full_pattern;
} /* grok_pattern_expand */

static void grok_capture_add_predicate(grok_t *grok, int capture_id,
                                       const char *predicate, int predicate_len) {
  grok_capture *gct;
  int offset = 0;

  grok_log(grok, LOG_PREDICATE, "Adding predicate '%.*s' to capture %d",
           predicate_len, predicate, capture_id);

  gct = (grok_capture *)grok_capture_get_by_id(grok, capture_id);
  if (gct == NULL) {
    grok_log(grok, LOG_PREDICATE, "Failure to find capture id %d", capture_id);
    return;
  }

  /* Compile the predicate into something useful */
  /* So far, predicates are just an operation and an argument */
  /* TODO(sissel): add predicate_func(capture_str, args) ??? */

  /* skip leading whitespace, use a loop since 'strspn' doesn't take a len */
  while (isspace(predicate[offset]) && offset < predicate_len) {
    offset++;
  }

  predicate += offset;
  predicate_len -= offset;

  if (predicate_len > 2) {
    if (!strncmp(predicate, "=~", 2) || !strncmp(predicate, "!~", 2)) {
      grok_predicate_regexp_init(grok, gct, predicate, predicate_len);
      return;
    } else if ((predicate[0] == '$') 
               && (strchr("!<>=", predicate[1]) != NULL)) {
      grok_predicate_strcompare_init(grok, gct, predicate, predicate_len);
      return;
    }
  } 
  if (predicate_len > 1) {
    if (strchr("!<>=", predicate[0]) != NULL) {
      grok_predicate_numcompare_init(grok, gct, predicate, predicate_len);
    }  else {
      fprintf(stderr, "Invalid predicate: '%.*s'\n", predicate_len, predicate);
    }
  } else {
    /* predicate_len == 1, here, and no 1-character predicates exist */
    fprintf(stderr, "Invalid predicate: '%.*s'\n", predicate_len, predicate);
  }

  /* update the database with our modified grok_capture */
  grok_capture_add(grok, gct);
}

static void grok_study_capture_map(grok_t *grok) {
  char *nametable;
  grok_capture *gct;
  int nametable_size;
  int nametable_entrysize;
  int i = 0;
  int offset = 0;
  int stringnum;
  int capture_id;

  pcre_fullinfo(grok->re, NULL, PCRE_INFO_NAMECOUNT, &nametable_size);
  pcre_fullinfo(grok->re, NULL, PCRE_INFO_NAMEENTRYSIZE, &nametable_entrysize);
  pcre_fullinfo(grok->re, NULL, PCRE_INFO_NAMETABLE, &nametable);

  for (i = 0; i < nametable_size; i++) {
    offset = i * nametable_entrysize;
    stringnum = (nametable[offset] << 8) + nametable[offset + 1];
    sscanf(nametable + offset + 2, CAPTURE_FORMAT, &capture_id);
    grok_log(grok, LOG_COMPILE, "Studying capture %d", capture_id);
    gct = (grok_capture *)grok_capture_get_by_id(grok, capture_id);
    assert(gct != NULL);
    gct->pcre_capture_number = stringnum;

    /* update the database with the new data */
    grok_capture_add(grok, gct);
  }
}

const char *grok_version() {
  return GROK_VERSION;
}
