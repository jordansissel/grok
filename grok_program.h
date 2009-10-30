#ifndef _GROK_PROGRAM_H_
#define _GROK_PROGRAM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <event.h>

//include "grok_input.h"
//include "grok_matchconf.h"

typedef struct grok_program grok_program_t;
typedef struct grok_collection grok_collection_t;
struct grok_input;
struct grok_matchconfig;

struct grok_program {
  char *name; /* optional program name */

  struct grok_input *inputs;
  int ninputs;
  int input_size;

  struct grok_matchconf *matchconfigs;
  int nmatchconfigs;
  int matchconfig_size;

  char **patternfiles;
  int npatternfiles;
  int patternfile_size;

  int logmask;
  int logdepth;

  grok_collection_t *gcol; /* if we are using this program in a collection */
  int reactions;
};

struct grok_collection {
  grok_program_t **programs; /* array of pointers to grok_program_t */
  int nprograms;
  int program_size;

  struct event_base *ebase;
  struct event *ev_sigchld;

  int logmask;
  int logdepth;
  int exit_code;
};

grok_collection_t *grok_collection_init();
void grok_collection_add(grok_collection_t *gcol, grok_program_t *gprog);
void grok_collection_loop(grok_collection_t *gcol);
void grok_collection_check_end_state(grok_collection_t *gcol);

#endif /* _GROK_PROGRAM_H_ */
