#!/bin/sh

. ./test.subr

rungrok() {
  perl ../grok -m "%SYSLOGDATE%" -r "%SYSLOGDATE|parsedate|strftime('&Y&m&d')%" < input/strftime
}

try_self $0
