#ifndef _PREDICATES_H_
#define _PREDICATES_H_

#include <pcre.h>
#include "grok.h"

/* Regular Expression Predicate
 * Activate with operator '=~'
 */

int grok_predicate_regexp_init(grok_t *grok, grok_capture *gct,
                               const char *args, int args_len);
int grok_predicate_numcompare_init(grok_t *grok, grok_capture *gct,
                                   const char *args, int args_len);
int grok_predicate_strcompare_init(grok_t *grok, grok_capture *gct,
                                   const char *args, int args_len);


#endif /* _PREDICATES_H_ */
