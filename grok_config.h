#include "grok_program.h"

#define CURPROGRAM (conf->programs[conf->nprograms - 1])
#define CURINPUT (CURPROGRAM.inputs [CURPROGRAM.ninputs - 1])
#define CURMATCH (CURPROGRAM.matchconfigs [CURPROGRAM.nmatchconfigs - 1])
#define CURPATTERNFILE (CURPROGRAM.patternfiles [CURPROGRAM.npatternfiles - 1])

#define SETLOG(parent, mine) \
  (mine).logmask = (parent).logmask;

  //(mine).logmask = ((mine).logmask == 0) ? (parent).logmask : (mine).logmask

struct config {
  grok_program_t *programs;
  int nprograms;
  int program_size;

  int logmask;
  int logdepth;
};

void conf_init(struct config *conf);
void conf_new_program(struct config *conf);
void conf_new_input(struct config *conf);
void conf_new_input_process(struct config *conf, char *cmd);
void conf_new_input_file(struct config *conf, char *filename);
void conf_new_matchconf(struct config *conf);
void conf_new_match_pattern(struct config *conf, const char *pattern);
void conf_match_set_debug(struct config *conf, int logmask);
