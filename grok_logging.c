#include <stdio.h>
#include <stdarg.h>
#include "grok.h"

#ifndef NOLOGGING
inline void _grok_log(int level, int indent, const char *format, ...) {
  va_list args;
  FILE *out;

  out = stderr;

  va_start(args, format);
  char *prefix;

  /* TODO(sissel): use gperf instead of this silly switch */
  switch (level) {
    case LOG_CAPTURE: prefix = "[capture] "; break;
    case LOG_COMPILE: prefix = "[compile] "; break;
    case LOG_EXEC: prefix = "[exec] "; break;
    case LOG_MATCH: prefix = "[match] "; break;
    case LOG_PATTERNS: prefix = "[patterns] "; break;
    case LOG_PREDICATE: prefix = "[predicate] "; break;
    case LOG_PROGRAM: prefix = "[program] "; break;
    case LOG_PROGRAMINPUT: prefix = "[programinput] "; break;
    case LOG_REACTION: prefix = "[reaction] "; break;
    case LOG_REGEXPAND: prefix = "[regexpand] "; break;
    case LOG_DISCOVER: prefix = "[discover] "; break;
    default: prefix = "[unknown] ";
  }
  fprintf(out, "[%d] %*s%s", getpid(), indent * 2, "", prefix);
  vfprintf(out, format, args);
  fprintf(out, "\n");
  va_end(args);
}
#endif
