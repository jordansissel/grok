#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "stringhelper.h"

void string_escape_like_c(char c, char *replstr, int *replstr_len, int *op);
void string_escape_hex(char c, char *replstr, int *replstr_len, int *op);
void string_escape_unicode(char c, char *replstr, int *replstr_len, int *op);

static char all_chars[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
            50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66,
            67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
            84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
            100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
            113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
            126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
            139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151,
            152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
            165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
            178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
            191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
            204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216,
            217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
            230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
            243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };

/* replace a string range with another string.
 *
 * if replace_len < 0, it is calculated from 'replace'
 * if strp_len < 0, it is calculated from *strp
 * if end < 0, it is set to start
 */
void substr_replace(char **strp, 
                    int *strp_len, int *strp_alloc_size,
                    int start, int end, 
                    const char *replace, int replace_len) {
  int total_len = 0;

  if (replace_len < 0)
    replace_len = strlen(replace);
  if (*strp_len < 0)
    *strp_len = strlen(*strp);
  if (start < 0) /* allow negative offset from end of string */
    start = *strp_len + start;
  if (end == 0) /* end of 0 means you want to copy the start value */
    end = start;
  else if (end < 0) /* negative values are offsets from end of string */
    end = *strp_len + end;

  total_len = *strp_len + replace_len - (end - start);

  if (total_len >= *strp_alloc_size) {
    *strp_alloc_size = total_len + 4096; /* grow by 4K + len */
    *strp = realloc(*strp, *strp_alloc_size);
  }

  memmove(*strp + start + replace_len,
          *strp + end,
          *strp_len - end);
  memcpy(*strp + start, replace, replace_len);
  *strp_len = start + (*strp_len - end)+  replace_len;
  (*strp)[*strp_len] = '\0';
}

#define ESCAPE_INSERT 1
#define ESCAPE_REPLACE 2

/* Escape a set of characters in a string */
void string_escape(char **strp, int *strp_len, int *strp_alloc_size,
                   const char *chars, int chars_len, int options) {
  int i = 0, j = 0, op = 0, replstr_len = 0;
  char replstr[8]; /* 7 should be enough (covers \uXXXX + null) */
  unsigned char c;
  //int hits_total = 0;
  unsigned char hits[256]; /* track chars found in the string */
  memset(hits, 0, 256);

  //printf("string_escape(\"%.*s\", %d, %d, \"%.*s\", %d, %d)\n",
         //*strp_len, *strp, *strp_len, *strp_alloc_size, chars_len, chars, 
         //chars_len, options);

  if (chars_len < 0) {
    chars_len = strlen(chars);
  }

  if (chars_len == 0) {
    chars_len = 256;
    chars = all_chars;
  }

  /* Make a map of characters found in the string */
  for (i = 0; i < *strp_len; i++) {
    c = (*strp)[i];
    hits[c] = 1;
  }


  for (i = 0; i < chars_len; i++) {
    c = chars[i];
    if (hits[c] == 0) {
      /* If this char is not in the string, skip it */
      continue;
    }

    if (options & ESCAPE_NONPRINTABLE && isprint(c)) {
      continue;
    }
    
    for (j = 0; j < *strp_len; j++) {
      //printf("%d / %c: %d\n", c, c, hits[(unsigned char)c]);
      /* chars are signed, so > 127 == -128 and such, which is an
       * invalid array offset in this case; cast to unsigned.  */
      if ((*strp)[j] != c) {
        continue;
      }

      //printf("Str: %.*s\n", strp_len, *strp);
      //printf("     %*s^\n", j, "");
      
      replstr_len = 0;
      op = ESCAPE_REPLACE; /* default operation */
      if (replstr_len == 0 && options & ESCAPE_LIKE_C) {
        string_escape_like_c(c, replstr, &replstr_len, &op);
      } 
      if (replstr_len == 0 && options & ESCAPE_UNICODE) {
        string_escape_unicode(c, replstr, &replstr_len, &op);
      }
      if (replstr_len == 0 && options & ESCAPE_HEX) {
        string_escape_hex(c, replstr, &replstr_len, &op);
      }

      if (replstr_len > 0) {
        switch (op) {
          case ESCAPE_INSERT:
            substr_replace(strp, strp_len, strp_alloc_size, j, j,
                           replstr, replstr_len);
            break;
          case ESCAPE_REPLACE:
            substr_replace(strp, strp_len, strp_alloc_size, j, j + replstr_len - 1,
                           replstr, replstr_len);
            break;
        }
      }

      j += replstr_len;
    }
  }
} /* void string_escape */

void string_escape_like_c(char c, char *replstr, int *replstr_len, int *op) {
  char *r = NULL;

  /* XXX: This should check iscntrl, instead, probably... */
  if (isprint(c)) {
    *op = ESCAPE_INSERT;
    r = "\\";
    *replstr_len = 1;
  } else {
    *op = ESCAPE_REPLACE;
    switch (c) {
      case '\n': r = "\\n"; break;
      case '\r': r = "\\r"; break;
      case '\b': r = "\\b"; break;
      case '\f': r = "\\f"; break;
      case '\t': r = "\\t"; break;
      case '\a': r = "\\a"; break;
    }
    if (r) {
      *replstr_len = 2;
    } else {
      *replstr_len = 0;
    }
  }
  memcpy(replstr, r, *replstr_len);
} /* void string_escape_like_c */

void string_escape_hex(char c, char *replstr, int *replstr_len, int *op) {
  *op = ESCAPE_REPLACE;
  *replstr_len = sprintf(replstr, "\\x%x", (unsigned char) c);
  //printf("Unicode: %.*s\n", *replstr_len, replstr);
} /* void string_escape_hex */

void string_escape_unicode(char c, char *replstr, int *replstr_len, int *op) {
  /* XXX: We should check the options to see if we should only convert
   * nonprintables */
  if (!isprint(c)) {
    *op = ESCAPE_REPLACE;
    *replstr_len = sprintf(replstr, "\\u00%02x",(unsigned char) c);
    //printf("Unicode: %.*s\n", *replstr_len, replstr);
  }
} /* string_escape_unicode */

void string_unescape(char **strp, int *strp_len, int *strp_size) {
  int i;
  char *repl;
  int repl_len;
  int orig_len;

  for (i = 0; i < *strp_len; i++) {
    repl_len = 0;
    orig_len = 0;
    if ((*strp)[i] == '\\') {
      switch ((*strp)[i + 1]) {
        case 't': repl = "\t"; repl_len = 1; orig_len = 2; break;
        case 'n': repl = "\n"; repl_len = 1; orig_len = 2; break;
        case 'b': repl = "\b"; repl_len = 1; orig_len = 2; break;
        case 'r': repl = "\r"; repl_len = 1; orig_len = 2; break;
        case 'a': repl = "\a"; repl_len = 1; orig_len = 2; break;
        case 'f': repl = "\f"; repl_len = 1; orig_len = 2; break;
        case 'v': repl = "\v"; repl_len = 1; orig_len = 2; break;
        case '"': repl = "\""; repl_len = 1; orig_len = 2; break;
        /* XXX: Handle octal? \0888 */
        /* XXX: Handle hex? \xFFFF */
        /* XXX: Handle unicode? \uFFFF */ 
      }

      if (repl_len > 0) {
        substr_replace(strp, strp_len, strp_size, i, i + orig_len,
                       repl, repl_len);
      }
    }
  }
} /* void string_unescape */

/* Some platforms don't have strndup, so let's provide our own */
char *string_ndup(const char *src, size_t size) {
  size_t len = 0;
  char *dup;

  while (src[len] != '\0' && len < size)
    len++;

  dup = malloc(len + 1);
  if (dup) {
    /* XXX: Should we use strncpy here, instead of memcpy? */
    memcpy(dup, src, len);
    dup[len] = '\0';
  }

  return dup;
} /* char *string_ndup */

int string_count(const char *src, const char *charlist) {
  string_ncount(src, strlen(src), charlist, strlen(charlist));
} /* int string_count */

int string_ncount(const char *src, size_t srclen,
                  const char *charlist, size_t listlen) {
  int i = 0, j = 0;
  int count = 0;

  for (i = 0; i < srclen; i++) {
    for (j = 0; j < listlen; j++) {
      if (src[i] == charlist[j]) {
        count++;
        break; /* no point in trying to match more of charlist this round */
      }
    }
  }
  return count;
} /* int string_ncount */
