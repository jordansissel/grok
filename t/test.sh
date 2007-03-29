#!/bin/sh

. ./test.subr

TESTS=
if [ $# -gt 0 ]; then 
  TESTS="$@"
else
  TESTS="shellquoting customquoting matchtest thresholds"
fi

set -- $TESTS
while [ $# -gt 0 ]; do
  try $1
  shift
done
