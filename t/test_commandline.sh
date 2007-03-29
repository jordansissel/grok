#!/bin/sh

. ./test.subr

rungrok() {
  perl ../grok -m "FOO" -r "%=LINE% BAR" < input/commandline
}

try commandline
