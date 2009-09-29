#!/bin/sh

#TESTS=`nm $1 | awk '$2 == "T" { printf("CU_add_test(suite, \"%s\", %s);\n", $3, $3) }'`
TESTS=`grep 'void test_' $1 | tr '(' ' ' | awk '{printf("CU_add_test(suite, \"%s:%s\", %s);\n", "'$1'", $2, $2) }'`

cat << EOF
#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"

#include "$1"

int main(int argc, char **argv) {
  CU_pSuite suite = NULL;

  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  suite = CU_add_suite("$1", NULL, NULL);
  if (NULL == suite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  $TESTS

  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
EOF

