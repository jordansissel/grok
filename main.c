#define _GNU_SOURCE
#include "grok.h"
#include "grok_program.h"
#include "grok_config.h"

#include "conf.tab.h"
#include <unistd.h>
#include <getopt.h>

extern char *optarg; /* from unistd.h, getopt */
extern FILE *yyin; /* from conf.lex (flex provides this) */

static char *g_prog;

void usage() {
  printf("Usage: %s [-d] <-f config_file>\n", g_prog);
  printf("       -d        (optional) Daemonize/background\n");
  printf("       -f file   (required) Use specified config file\n");
}

int main(int argc, char **argv) {
  struct config c;
  grok_collection_t *gcol = NULL;
  int i = 0;
  int opt = 0;

  int want_daemon = 0;
  char *config_file = NULL;

  struct option options[] = {
    { "daemon", no_argument, NULL, 'd' },
    { "config", required_argument, NULL, 'f' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0 }
  };

  g_prog = argv[0];

  while ((opt = getopt_long_only(argc, argv, "hdf:", options, &optind)) != -1) {
    switch (opt) {
      case 'd':
        want_daemon = 1;
        break;
      case 'f':
        config_file = strdup(optarg);
        break;
      case 'h':
        usage();
        return 0;
      default:
        usage();
        return 1;
    }
  }

  if (config_file == NULL) {
    fprintf(stderr, "No config file given.\n");
    usage();
    return 1;
  }

  yyin = fopen(config_file, "r");
  free(config_file);
  conf_init(&c);
  i = yyparse(&c);
  if (i != 0) {
    fprintf(stderr, "Parsing error in config file\n");
    return 1;
  }

  /* We want to background after parsing the config so we can check for syntax
   * errors */
  if (want_daemon && (daemon(0, 0) != 0)) {
      perror("Error while daemonizing");
  }

  gcol = grok_collection_init();
  for (i = 0; i < c.nprograms; i++) {
    grok_collection_add(gcol, &(c.programs[i]));
  }
  grok_collection_loop(gcol);

  return gcol->exit_code;
}
