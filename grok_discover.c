#include <grok.h>
#include "stringhelper.h"

static int dgrok_init = 0;
static grok_t global_discovery_req1_grok;
static grok_t global_discovery_req2_grok;
static int complexity(const grok_t *grok);

static void grok_discover_global_init() {
  dgrok_init = 1;
  grok_init(&global_discovery_req1_grok);
  grok_compile(&global_discovery_req1_grok, ".\\b.");

  grok_init(&global_discovery_req2_grok);
  grok_compile(&global_discovery_req2_grok, "%\\{[^}]+\\}");
}

grok_discover_t *grok_discover_new(grok_t *source_grok) {
  grok_discover_t *gdt = malloc(sizeof(grok_discover_t));
  grok_discover_init(gdt, source_grok);
  return gdt;
}

void grok_discover_init(grok_discover_t *gdt, grok_t *source_grok) {
  TCLIST *names = NULL;
  int i = 0, len = 0;

  if (dgrok_init == 0) {
    grok_discover_global_init();
  }

  gdt->complexity_tree = tctreenew2(tccmpint32, NULL);
  gdt->base_grok = source_grok;
  gdt->logmask = source_grok->logmask;
  gdt->logdepth = source_grok->logdepth;

  names = grok_pattern_name_list(source_grok);
  len = tclistnum(names);
  /* for each pattern, create a grok. 
   * Sort by complexity.
   * loop
   *   for each pattern, try replacement
   *   if no replacements, break
   */
  for (i = 0; i < len; i++) {
    int namelen = 0;
    const char *name = tclistval(names, i, &namelen);

    int *key = malloc(sizeof(int));
    grok_t *g = grok_new();
    grok_clone(g, source_grok);
    char *gpattern;
    if (asprintf(&gpattern, "%%{%.*s}", namelen, name) == -1) {
      perror("asprintf failed");
      abort();
    }
    grok_compile(g, gpattern);
    *key = complexity(g);

    /* Low complexity should be skipped */
    if (*key > -20) {
      free((void *)g->pattern);
      free(key);
      grok_free_clone(g);
      free(g);
      continue;
    }

    grok_log(gdt, LOG_DISCOVER, "Including pattern: (complexity: %d) %.*s",
             *(int *)key, namelen, name);
    tctreeput(gdt->complexity_tree, key, sizeof(int), g, sizeof(grok_t));
    //grok_free_clone(g);
    //free(key);
  }

  tclistdel(names);
}

void grok_discover_clean(grok_discover_t *gdt) {
  tctreedel(gdt->complexity_tree);
  gdt->base_grok = NULL;
}

void grok_discover_free(grok_discover_t *gdt) {
  grok_discover_clean(gdt);
  free(gdt);
}

void grok_discover(const grok_discover_t *gdt, /*grok_t *dest_grok, */
                   const char *input, char **discovery, int *discovery_len) {
  /* Find known patterns in the input string */
  ((grok_discover_t *)gdt)->logmask = LOG_DISCOVER;
  char *pattern = strdup(input);
  int pattern_len = strlen(pattern);
  int pattern_size = pattern_len + 1;
  int replacements = -1;
  //int offset = 0;
  int match = 0;
  char *cursor = pattern;
  int rounds = 0;
  while (replacements != 0) {
    const void *key;
    int key_len;
    rounds++;
    replacements = 0;
    grok_log(gdt, LOG_DISCOVER, "%d: Round starting\n", rounds);
    tctreeiterinit(gdt->complexity_tree);
    grok_match_t gm;
    grok_match_t best_match;
    int max_matchlen = 0;
    //char *patoff = pattern + offset;
    while ((key = tctreeiternext(gdt->complexity_tree, &key_len)) != NULL) {
      const int *complexity = (const int *)key;
      int val_len;
      const grok_t *g = tctreeget(gdt->complexity_tree, key, sizeof(int), &val_len);
      match = grok_exec(g, pattern, &gm);
      if (match == GROK_OK) {
        int still_ok;
        int matchlen = gm.end - gm.start;
        still_ok = grok_execn(&global_discovery_req1_grok, pattern + gm.start,
                              matchlen, NULL);
        if (still_ok != GROK_OK) {
          grok_log(gdt, LOG_DISCOVER, 
                   "%d: Matched %s, but match (%.*s) not complex enough.",
                   rounds, g->pattern, matchlen, pattern + gm.start);
          continue;
        }

        /* We don't want to replace existing patterns like %{FOO} */
        if (grok_execn(&global_discovery_req2_grok, pattern + gm.start, matchlen,
                       NULL) == GROK_OK) {
          grok_log(gdt, LOG_DISCOVER, 
                   "%d: Matched %s, but match (%.*s) includes %{...} patterns.",
                   rounds, g->pattern, matchlen, pattern + gm.start);
          continue;
        }

        /* A longer match is a better match.
         * If match length is equal to max, then still take this match as
         * better since if true, then this match has a pattern that is less
         * complex and is therefore a more relevant match */
        if (max_matchlen <= matchlen) {
          grok_log(gdt, LOG_DISCOVER,
                   "%d: New best match: %s", rounds, g->pattern);
          max_matchlen = matchlen;
          memcpy(&best_match, &gm, sizeof(grok_match_t));
        }
      } /* match == GROK_OK */
    } /* tctreeiternext(complexity_tree ...) */

    if (max_matchlen > 0) {
      grok_log(gdt, LOG_DISCOVER, "%d: Matched %s on '%.*s'",
               rounds, best_match.grok->pattern,
               best_match.end - best_match.start, pattern + best_match.start);
      replacements = 1;
      substr_replace(&pattern, &pattern_len, &pattern_size,
                     best_match.start, best_match.end,
                     best_match.grok->pattern, best_match.grok->pattern_len);
      substr_replace(&pattern, &pattern_len, &pattern_size,
                     best_match.start, best_match.start, "\\E", 2);
      substr_replace(&pattern, &pattern_len, &pattern_size,
                     best_match.start + best_match.grok->pattern_len + 2, -1,
                     "\\Q", 2);
      //usleep(1000000);
      /* Wrap the new regexp in \E .. \Q, for ending and beginning (respectively)
       * 'quote literal' as PCRE and Perl support. This prevents literal characters
       * in the input strings from being interpreted */
      //grok_log(gdt, LOG_DISCOVER, "%d: Pattern1: %.*s", rounds, pattern_len, pattern);
      grok_log(gdt, LOG_DISCOVER, "%d: Pattern3: %.*s", rounds, pattern_len, pattern);
    }
  } /* while (replacements != 0) */

  /* Add \Q and \E at beginning and end */
  substr_replace(&pattern, &pattern_len, &pattern_size,
                 0, 0, "\\Q", 2);
  substr_replace(&pattern, &pattern_len, &pattern_size,
                 pattern_len, pattern_len, "\\E", 2);
  *discovery = pattern;
  *discovery_len = pattern_len;
}

/* Compute the relative complexity of a pattern */
static int complexity(const grok_t *grok) {
  int score;
  score += string_count(grok->full_pattern, "|");
  score += strlen(grok->full_pattern) / 2;
  return -score; /* Sort most-complex first */
}
