#!/bin/sh

. ./test.subr

rungrok() {
  perl ../grok -m "%WORD:TEST%" -r "%WORD:TEST%" < input/subnames
}

try subnames
