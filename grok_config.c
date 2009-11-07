#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tcutil.h>

#include "grok_input.h"
#include "grok_config.h"
#include "grok_matchconf.h"
#include "grok_logging.h"

void conf_init(struct config *conf) {
  conf->nprograms = 0;
  conf->program_size = 10;
  conf->programs = calloc(conf->program_size, sizeof(grok_pattern_t));
  conf->logmask = 0;
  conf->logdepth = 0;
}

void conf_new_program(struct config *conf) {
  /* TODO(sissel): put most of this into grok_program_init, or something */
  conf->nprograms++;
  if (conf->nprograms == conf->program_size) {
    conf->program_size *= 2;
    conf->programs = realloc(conf->programs,
                             conf->program_size * sizeof(grok_pattern_t));
  }

  CURPROGRAM.ninputs = 0;
  CURPROGRAM.input_size = 10;
  CURPROGRAM.inputs = calloc(CURPROGRAM.input_size, sizeof(grok_input_t));

  CURPROGRAM.nmatchconfigs = 0;
  CURPROGRAM.matchconfig_size = 10;
  CURPROGRAM.matchconfigs = calloc(CURPROGRAM.matchconfig_size,
                                    sizeof(grok_matchconf_t));

  CURPROGRAM.npatternfiles = 0;
  CURPROGRAM.patternfile_size = 10;
  CURPROGRAM.patternfiles = calloc(CURPROGRAM.patternfile_size, sizeof(char *));

  CURPROGRAM.reactions = 0;

  //CURPROGRAM.logmask = ~0;

  SETLOG(*conf, CURPROGRAM);
}

void conf_new_patternfile(struct config *conf) {
  CURPROGRAM.npatternfiles++;
  if (CURPROGRAM.npatternfiles == CURPROGRAM.patternfile_size) {
    CURPROGRAM.patternfile_size *= 2;
    CURPROGRAM.patternfiles = realloc(CURPROGRAM.patternfiles,
                                      CURPROGRAM.patternfile_size * sizeof(char *));
  }
}

void conf_new_input(struct config *conf) {
  /* Bump number of programs and grow our programs array if necessary */
  CURPROGRAM.ninputs++;
  if (CURPROGRAM.ninputs == CURPROGRAM.input_size) {
    CURPROGRAM.input_size *= 2;
    CURPROGRAM.inputs = realloc(CURPROGRAM.inputs,
                                 CURPROGRAM.input_size * sizeof(grok_input_t));
  }

  /* initialize the new input */
  memset(&CURINPUT, 0, sizeof(grok_input_t));
  SETLOG(CURPROGRAM, CURINPUT);
}

void conf_new_input_process(struct config *conf, char *cmd) {
  conf_new_input(conf);
  CURINPUT.type = I_PROCESS;
  CURINPUT.source.process.cmd = cmd;
}

void conf_new_input_file(struct config *conf, char *filename) {
  conf_new_input(conf);
  CURINPUT.type = I_FILE;
  CURINPUT.source.file.filename = filename;
}

void conf_new_matchconf(struct config *conf) {
  CURPROGRAM.nmatchconfigs++;
  if (CURPROGRAM.nmatchconfigs == CURPROGRAM.matchconfig_size) {
    CURPROGRAM.matchconfig_size *= 2;
    CURPROGRAM.matchconfigs = realloc(CURPROGRAM.matchconfigs,
                                      CURPROGRAM.matchconfig_size 
                                      * sizeof(grok_matchconf_t));
  }

  grok_matchconfig_init(&CURPROGRAM, &CURMATCH);

  /* Set sane defaults. */
  CURMATCH.reaction = "%{@LINE}";
  CURMATCH.shell = "stdout";
}

void conf_new_match_pattern(struct config *conf, const char *pattern) {
  int compile_ret;
  grok_t *grok;
  grok = malloc(sizeof(grok_t));
  grok_init(grok);

  int i = 0;
  for (i = 0; i < CURPROGRAM.npatternfiles; i++) {
    grok_patterns_import_from_file(grok, CURPROGRAM.patternfiles[i]);
  }
  SETLOG(CURPROGRAM, *grok);

  compile_ret = grok_compile(grok, pattern);
  if (compile_ret != GROK_OK) {
    fprintf(stderr, "Failure compiling pattern '%s': %s\n",
            pattern, grok->errstr);
    exit(1);
  }

  tclistpush(CURMATCH.grok_list, grok, sizeof(grok_t));
}

void conf_match_set_debug(struct config *conf, int logmask) {
  int i = 0;
  int listsize = tclistnum(CURMATCH.grok_list);
  int unused_len;

  for (i = 0; i < listsize; i++) {
    grok_t *grok;
    grok = (grok_t *)tclistval(CURMATCH.grok_list, i, &unused_len);
    grok->logmask = logmask;
    tclistover(CURMATCH.grok_list, i, grok, sizeof(grok_t));
  }
}

