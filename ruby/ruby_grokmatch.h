#ifndef _RUBY_GROKMATCH_H_
#define  _RUBY_GROKMATCH_H_

#include "rgrok.h"
#include <grok.h>

#ifdef _IS_RUBY_GROKMATCH_
  #define CONDEXTERN
#else
  #define CONDEXTERN extern
#endif

CONDEXTERN VALUE rGrokMatch_new_from_grok_match(grok_match_t *gm);
#endif /*  _RUBY_GROKMATCH_H_ */
