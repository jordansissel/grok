#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "grok.h"

#define LOG_PREDICATE (1 << 0)
#define LOG_COMPILE (1 << 1)
#define LOG_EXEC (1 << 2)
#define LOG_REGEXPAND (1 << 3)
#define LOG_PATTERNS (1 << 4)
#define LOG_MATCH (1 << 5)
#define LOG_CAPTURE (1 << 6)
#define LOG_PROGRAM (1 << 7)
#define LOG_PROGRAMINPUT (1 << 8)
#define LOG_REACTION (1 << 9)
#define LOG_DISCOVER (1 << 10)

#define LOG_ALL (~0)

#ifdef NOLOGGING
/* this 'args...' requires GNU C */
#  define grok_log(obj, level, format, args...) { }
#else

void _grok_log(int level, int indent, const char *format, ...);

/* let us log anything that has both a 'logmask' and 'logdepth' member */
#  define grok_log(obj, level, format, args...) \
  if ((obj)->logmask & level) \
    _grok_log(level, (obj)->logdepth, "[%s:%d] " format, \
              __FUNCTION__, __LINE__, ## args)

#endif

#endif /* _LOGGING_H_ */
