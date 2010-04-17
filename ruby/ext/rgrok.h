#ifndef _RGROK_H_
#define _RGROK_H_ 
#include <ruby.h>
#include <grok.h>

extern VALUE rGrok_new(VALUE klass);
extern VALUE rGrok_new_from_grok(grok_t *grok);

#endif /* _RGROK_H_ */
