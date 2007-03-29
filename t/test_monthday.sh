#!/bin/sh

. ./test.subr

which jot > /dev/null 2>&1 && JOT="jot 31 1; jot 10 35"
which seq > /dev/null 2>&1 && JOT="seq 31; seq 35 45"

if [ -z "$JOT" ]; then
  echo "No sequence generation tool found (jot or seq). Skipping $0";
  exit 1
fi

rungrok() {
  sh -c "$JOT" | perl ../grok -m '^%MONTHDAY%$' -r "%MONTHDAY%"
}

#rungrok
try_self $0
