
#include <stdio.h>
#include <string.h>

#include "grok_logging.h"
#include "predicates.h"

static pcre *regexp_predicate_op = NULL;
#define REGEXP_PREDICATE_RE \
  "(?:\\s*([!=])~" \
  "\\s*" \
  "(.)" \
    "([^\\/]+|(?:\\/)+)*)" \
  "(?:\\g{-2})"

static void grok_predicate_regexp_global_init(void);

/* Operation things */

typedef enum { OP_LT, OP_GT, OP_GE, OP_LE, OP_EQ, OP_NE } operation;
int strop(const char * const args, int args_len);

/* Return length of operation in string. ie; "<=" (OP_LE) == 2 */
#define OP_LEN(op) ((op == OP_GT || op == OP_LT) ? 1 : 2)

/* grok predicates should return 0 for success and 1 for failure.
 * normal comparison (like 3 < 4) returns 1 for success, and 0 for failure. 
 * So we negate the comparison return value here. */
#define OP_RUN(op, cmpval, retvar) \
    switch (op) { \
      case OP_LT: retvar = !(cmpval < 0); break; \
      case OP_GT: retvar = !(cmpval > 0); break; \
      case OP_GE: retvar = !(cmpval >= 0); break; \
      case OP_LE: retvar = !(cmpval <= 0); break; \
      case OP_EQ: retvar = !(cmpval == 0); break; \
      case OP_NE: retvar = !(cmpval != 0); break; \
    } 


typedef struct grok_predicate_regexp {
  //pcre *re;
  grok_t gre;
  char *pattern;
  int negative_match;
} grok_predicate_regexp_t;

typedef struct grok_predicate_numcompare {
  enum { DOUBLE, LONG } type;
  operation op;
  union {
    long lvalue;
    double dvalue;
  } u;
} grok_predicate_numcompare_t;

typedef struct grok_predicate_strcompare {
  operation op;
  char *value;
  int len;
} grok_predicate_strcompare_t;

int grok_predicate_regexp(grok_t *grok, const grok_capture *gct,
                          const char *subject, int start, int end);
int grok_predicate_numcompare(grok_t *grok, const grok_capture *gct,
                              const char *subject, int start, int end);
int grok_predicate_strcompare(grok_t *grok, const grok_capture *gct,
                              const char *subject, int start, int end);

static void grok_predicate_regexp_global_init(void) {
  if (regexp_predicate_op == NULL) {
    int erroffset = -1;
    const char *errp;
    regexp_predicate_op = pcre_compile(REGEXP_PREDICATE_RE, 0, 
                                       &errp, &erroffset, NULL);
    if (regexp_predicate_op == NULL) {
      fprintf(stderr, "Internal error (compiling predicate regexp op): %s\n",
              errp);
    }
  }
}

int grok_predicate_regexp_init(grok_t *grok, grok_capture *gct,
                               const char *args, int args_len) {
  #define REGEXP_OVEC_SIZE 6
  int capture_vector[REGEXP_OVEC_SIZE * 3];
  int ret; 

  grok_log(grok, LOG_PREDICATE, "Regexp predicate found: '%.*s'", args_len, args);

  grok_predicate_regexp_global_init();
  ret = pcre_exec(regexp_predicate_op, NULL, args, args_len, 0, 0,
                  capture_vector, REGEXP_OVEC_SIZE * 3);
  if (ret < 0) {
    fprintf(stderr, "An error occurred in grok_predicate_regexp_init.\n");
    fprintf(stderr, "Args: %.*s\n", args_len, args);
    fprintf(stderr, "pcre_exec:: %d\n", ret);
    return 1;
  }

  int start, end;
  grok_predicate_regexp_t *gprt;
  start = capture_vector[6]; /* capture #3 */
  end = capture_vector[7];

  gprt = calloc(1, sizeof(grok_predicate_regexp_t));
  gprt->pattern = calloc(1, end - start + 1);
  strncpy(gprt->pattern, args + start, end - start);
  //gprt->re = pcre_compile(gprt->pattern, 0, &errptr, &erroffset, NULL);

  grok_log(grok, LOG_PREDICATE, "Regexp predicate is '%s'", gprt->pattern);
  grok_clone(&gprt->gre, grok);
  ret = grok_compile(&gprt->gre, gprt->pattern);

  gprt->negative_match = (args[capture_vector[2]] == '!');

  if (ret != 0) {
    fprintf(stderr, "An error occurred while compiling the predicate for %s:\n",
            gct->name);
    fprintf(stderr, "Error at pos %d: %s\n",
            grok->pcre_erroffset, grok->pcre_errptr);
    return 1;
  }

  grok_log(grok, LOG_PREDICATE, 
           "Compiled %sregex for '%s': '%s'", 
           (gprt->negative_match) ? "negative match " : "",
           gct->name, gprt->pattern);
  /* strdup here and be lazy. Otherwise, we'll have to add a new member
   * to grok_capture which indicates which fields of it are set to 
   * non-heap pointers. */

  /* Break const... */
  gct->predicate_func_name = strdup("grok_predicate_regexp");
  gct->predicate_func_name_len = strlen("grok_predicate_regexp");
  grok_capture_set_extra(grok, gct, gprt);
  grok_capture_add(grok, gct);

  return 0;
}

int grok_predicate_regexp(grok_t *grok, const grok_capture *gct,
                          const char *subject, int start, int end) {
  grok_predicate_regexp_t *gprt; /* XXX: grok_capture extra */
  int ret;

  gprt = *(grok_predicate_regexp_t **)(gct->extra.extra_val);
  ret = grok_execn(&gprt->gre, subject + start, end - start, NULL);
  
  grok_log(grok, LOG_PREDICATE, "RegexCompare: grok_execn returned %d", ret);

  /* negate the match if necessary */
  if (gprt->negative_match) {
    switch(ret) {
      case GROK_OK: ret = GROK_ERROR_NOMATCH; break;
      case GROK_ERROR_NOMATCH: ret = GROK_OK; break;
    }
  } else {
    grok_log(grok, LOG_PREDICATE, "RegexCompare: PCRE error %d", ret);
  }

  grok_log(grok, LOG_PREDICATE, "RegexCompare: '%.*s' =~ /%s/ => %s",
           (end - start), subject + start, gprt->pattern,
           (ret < 0) ? "false" : "true");

  /* grok_execn returns GROK_OK for success. */
  /* pcre_callout expects:
   * 0 == ok, 
   * >=1 for 'fail but try another match'
   */
  switch(ret) {
    case GROK_OK:
      return 0;
      break;
    default:
      return 1;
  }
}

int grok_predicate_numcompare_init(grok_t *grok, grok_capture *gct,
                                   const char *args, int args_len) {
  grok_predicate_numcompare_t *gpnt;

  /* I know I said that args is a const char, but we need to modify the string
   * temporarily so that strtol and strtod don't overflow a buffer when they
   * don't see a terminator. */
  char *tmp = (char *)args;
  int pos;
  char a = args[args_len];

  grok_log(grok, LOG_PREDICATE, "Number compare predicate found: '%.*s'",
           args_len, args);

  gpnt = calloc(1, sizeof(grok_predicate_numcompare_t));

  gpnt->op = strop(args, args_len);
  pos = OP_LEN(gpnt->op);

  tmp[args_len] = 0; /* force null byte so strtol doesn't run wild */

  /* Optimize and use long type if the number is not a float (no period) */
  if (strchr(tmp, '.') == NULL) {
    gpnt->type = LONG;
    gpnt->u.lvalue = strtol(tmp + pos, NULL, 0);
    grok_log(grok, LOG_PREDICATE, "Arg '%.*s' is non-floating, assuming long type",
             args_len - pos, tmp + pos);
  } else {
    gpnt->type = DOUBLE;
    gpnt->u.dvalue = strtod(tmp + pos, NULL);
    grok_log(grok, LOG_PREDICATE, "Arg '%.*s' looks like a double, assuming double",
             args_len - pos, tmp + pos);
  }
  /* Restore the original character at the end, which probably wasn't a null byte */
  tmp[args_len] = a;

  gct->predicate_func_name = strdup("grok_predicate_numcompare");
  gct->predicate_func_name_len = strlen("grok_predicate_numcompare");
  grok_capture_set_extra(grok, gct, gpnt);
  grok_capture_add(grok, gct);
  return 0;
}

int grok_predicate_numcompare(grok_t *grok, const grok_capture *gct,
                              const char *subject, int start, int end) {
  grok_predicate_numcompare_t *gpnt;
  int ret = 0;

  gpnt = *(grok_predicate_numcompare_t **)(gct->extra.extra_val);

  if (gpnt->type == DOUBLE) {
    double a = strtod(subject + start, NULL);
    double b = gpnt->u.dvalue;
    OP_RUN(gpnt->op, a - b, ret);
    grok_log(grok, LOG_PREDICATE, "NumCompare(double): %f vs %f == %s (%d)",
             a, b, (ret) ? "false" : "true", ret);
  } else {
    long a = strtol(subject + start, NULL, 0);
    long b = gpnt->u.lvalue;
    OP_RUN(gpnt->op, a - b, ret);
    grok_log(grok, LOG_PREDICATE, "NumCompare(long): %ld vs %ld == %s (%d)",
             a, b, (ret) ? "false" : "true", ret);
  }

  return ret;
}

int grok_predicate_strcompare_init(grok_t *grok, grok_capture *gct,
                                   const char *args, int args_len) {
  grok_predicate_strcompare_t *gpst;
  int pos;

  grok_log(grok, LOG_PREDICATE, "String compare predicate found: '%.*s'",
           args_len, args);

  /* XXX: ALLOC */
  gpst = calloc(1, sizeof(grok_predicate_strcompare_t));

  /* skip first character, which is '$' */
  args++;
  args_len--;

  gpst->op = strop(args, args_len);
  pos = OP_LEN(gpst->op);
  pos += strspn(args + pos, " ");
  grok_log(grok, LOG_PREDICATE, "String compare rvalue: '%.*s'",
           args_len - pos, args + pos);

  /* XXX: ALLOC */
  gpst->len = args_len - pos;
  gpst->value = malloc(gpst->len);
  memcpy(gpst->value, args + pos, gpst->len);

  gct->predicate_func_name = strdup("grok_predicate_strcompare");
  gct->predicate_func_name_len = strlen("grok_predicate_strcompare");
  grok_capture_set_extra(grok, gct, gpst);
  grok_capture_add(grok, gct);

  return 0;
}

int grok_predicate_strcompare(grok_t *grok, const grok_capture *gct,
                              const char *subject, int start, int end) {
  grok_predicate_strcompare_t *gpst;
  int ret = 0;
   
  gpst = *(grok_predicate_strcompare_t **)(gct->extra.extra_val);

  OP_RUN(gpst->op,
         strncmp(subject + start, gpst->value, (end - start)),
         ret);

  grok_log(grok, LOG_PREDICATE, "Compare: '%.*s' vs '%.*s' == %s",
           (end - start), subject + start, gpst->len, gpst->value,
           (ret) ? "false" : "true");

  /* grok predicates should return 0 for success, 
   * but comparisons return 1 for success, so negate the comparison */
  return ret;
}

int strop(const char * const args, int args_len) {
  if (args_len == 0)
    return -1;
  
  switch (args[0]) {
    case '<':
      if (args_len >= 2 && args[1] == '=') return OP_LE;
      else return OP_LT;
      break;
    case '>':
      if (args_len >= 2 && args[1] == '=') return OP_GE;
      else return OP_GT;
      break;
    case '=':
      if (args_len >= 2 && args[1] == '=') return OP_EQ;
      else {
        fprintf(stderr, "Invalid predicate: '%.*s'\n", args_len, args);
        return -1;
      }
      break;
    case '!':
      if (args_len >= 2 && args[1] == '=') return OP_NE;
      else {
        fprintf(stderr, "Invalid predicate: '%.*s'\n", args_len, args);
        return -1;
      }
      break;
    default:
      fprintf(stderr, "Invalid predicate: '%.*s'\n", args_len, args);
  }

  return -1;
}
