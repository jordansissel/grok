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
  int opt = 0;

  struct option options[] = {
    { "patterns", required_argument, NULL, 'p' },
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
  };

  const char *prog = argv[0];
  grok_t grok;
  grok_init(&grok);

  while ((opt = getopt_long_only(argc, argv, "hp:v", options, &optind)) != -1) {
    switch (opt) {
      case 'h':
        usage();
        return 0;
      case 'p':
        grok_patterns_import_from_file(&grok, optarg);
        break;
      case 'v':
        grok.logmask =~ 0;
        break;
      default:
        usage();
        return 1;
    }
  }

  argc -= optind;
  argv += optind;

  int i;
  FILE *fp = stdin;
  if (argc > 0 && strcmp(argv[0], "-")) {
    fp = fopen(argv[0], "r");
  }

  char buf[4096];
  grok_discover_t *gdt;
  grok_t grok_copy;
  gdt = grok_discover_new(&grok);
  while (fgets(buf, 4096, fp) != NULL) {
    grok_init(&grok_copy);
    strrchr(buf, '\n')[0] = '\0';
    grok_discover(gdt, &grok_copy, buf);
    printf("%s\n", grok_copy.pattern);
    grok_free_clone(&grok_copy);
  }
  grok_discover_free(gdt);
  return 0;
}
