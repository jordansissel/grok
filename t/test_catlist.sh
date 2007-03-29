#!/bin/sh

. ./test.subr

compare() {
  f1=`mktemp /tmp/logwatcher.compare.XXXX`
  f2=`mktemp /tmp/logwatcher.compare.XXXX`
  sort $1 > $f1
  sort $2 > $f2
  diff -u $f1 $f2
  return $?
}

try catlist
