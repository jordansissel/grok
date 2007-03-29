#!/bin/sh

. ./test.subr

rungrok() {
  perl ../grok -m "%SYSLOGDATE%" -r "%SYSLOGDATE|parsedate%" < input/parsedate
}

try_self $0
