#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stringhelper.h"

void test_substr_replace_unmoving_insertion(void) {
  char *source = calloc(1, 1024);
  char *dest = calloc(1, 1024);
  int slen = -1, dlen = -1, dalloc = 1024;

  sprintf(source, "world");
  sprintf(dest, "hello there");

  substr_replace(&dest, &dlen, &dalloc, 6, strlen(dest), source, slen);
  CU_ASSERT(!strcmp(dest, "hello world"));
  CU_ASSERT(dalloc == 1024);
  CU_ASSERT(dlen == 11);

  free(source);
  free(dest);
}

void test_substr_replace_alloc_and_insert_from_null_dest(void) {
  char *source = "hello world";
  char *dest = NULL;
  int dlen = 0, dalloc = 0;

  substr_replace(&dest, &dlen, &dalloc, 0, 0, source, -1);

  CU_ASSERT(!strcmp(dest, source));
  CU_ASSERT(dlen == strlen(source));
  CU_ASSERT(dalloc > dlen);

  free(dest);
}

void test_substr_replace_remove(void) {
  char *source = strdup("hello world");
  int len = strlen(source);
  int size = len + 1;

  //printf("\n--\n%s\n", source);
  substr_replace(&source, &len, &size, 5, len, "", 0);
  //printf("\n--\n%s\n", source);
  CU_ASSERT(!strcmp(source, "hello"));
}

void test_substr_replace_lenchange(void) {
  char *source = strdup("hello world test");
  int len = strlen(source);
  int size = len + 1;

  CU_ASSERT(len == 16);
  substr_replace(&source, &len, &size, 5, len, "", 0);
  CU_ASSERT(len == 5);
}

void test_string_escape_c(void) {
  struct {
    char *input;
    char *output;
  } data[] = {
    { "no change", "no change" },
    { "quoty \" ?", "quoty \\\" ?" },
    { "test \n", "test \\n" },
    { "test \r", "test \\r" },
    { "test \f", "test \\f" },
    { "test \a", "test \\a" },
    { "test \b", "test \\b" },
    { "test \\", "test \\\\" },
    { NULL, NULL },
  };

  int i = 0;
  for (i = 0; data[i].input != NULL ; i++) {
    int len, size;
    char *s = strdup(data[i].input);
    len = strlen(s);
    size = len + 1;

    string_escape(&s, &len, &size, "\\\"", 2, ESCAPE_LIKE_C);
    string_escape(&s, &len, &size, "", 0, ESCAPE_LIKE_C | ESCAPE_NONPRINTABLE);

    //printf("\n");
    //printf("'%s' vs '%s' ('%s')\n", data[i].input, s, data[i].output);
    //printf("\n");
    if (strcmp(s, data[i].output)) {
      printf("\n");
      printf("'%s' vs '%s' ('%s')\n", data[i].input, s, data[i].output);
      printf("\n");
    }
    CU_ASSERT(!strcmp(s, data[i].output));

    free(s);
  }
}

void test_string_ndup(void) {
  char data[] = "hello there";
  char *p;

  p = string_ndup(data, 5);
  CU_ASSERT(!strcmp(p, "hello"));
}
